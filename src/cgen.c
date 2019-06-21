/****************************************************/
/* File: cgen.c                                     */
/* The code generator interface to                  */
/* the C-MINUS compiler                             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"
#include "util.h"

/* prototypes for code generation functions */
static void cgen(TreeNode *node);
static void cgenStmt(TreeNode *node);
static void cgenExp(TreeNode *node);
static void cgenOp(TreeNode *node);
static void cgenAssign(TreeNode *node);
static void cgenCompound(TreeNode *node);
static void cgenPop(const char *reg);
static void cgenPush(const char *reg);
static void cgenPrintString(const char *symbol);
static int getLabel(void);
static void cgenArrayAddress(TreeNode *node);
#define getName(node) (node->symbol->treeNode->attr.name)

static int returnLabel; /* return label used in a function */

const char *const argumentRegisters[] = {"$a0", "$a1", "$a2", "$a3"};

/* Procedure cgenStmt generates code at a statement node */
static void cgenStmt(TreeNode *node)
{
  switch (node->kind.stmt)
  {
  case CompoundK:
    cgenCompound(node);
    break;
  case SelectionK:
  {
    int followingLabel = getLabel();
    emitComment("->selection");
    cgenExp(node->child[0]);
    if (node->child[2])
    { /* Has else statement */
      int elseLabel = getLabel();
      emitRegLabel("beqz", "$v0", elseLabel);
      cgen(node->child[1]);
      emitLabel("b", followingLabel);
      emitLabelNum(elseLabel);
      cgen(node->child[2]);
    }
    else
    { /* No else statement */
      emitRegLabel("beqz", "$v0", followingLabel);
      cgen(node->child[1]);
    }
    emitLabelNum(followingLabel);
    emitComment("<-selection");
    break;
  }
  case IterationK:
  {
    int conditionLabel = getLabel();
    int followingLabel = getLabel();
    emitComment("->iteration");
    emitLabelNum(conditionLabel);
    cgenExp(node->child[0]);
    emitRegLabel("beqz", "$v0", followingLabel);
    cgen(node->child[1]);
    emitLabel("b", conditionLabel);
    emitLabelNum(followingLabel);
    emitComment("<-iteration");
    break;
  }
  case ReturnK:
    cgenExp(node->child[0]);
    emitLabel("j", returnLabel);
    break;
  }
} /* cgenStmt */

/* Procedure cgenExp generates code at an expression node
 * Value of expression will be in $v0 */
static void cgenExp(TreeNode *node)
{
  switch (node->kind.exp)
  {
  case AssignK:
    cgenAssign(node);
    break;
  case OpK:
    cgenOp(node);
    break;
  case ConstK:
    emitComment("->Const");
    emitRegImm("li", "$v0", node->attr.val);
    emitComment("<-Const");
    break;
  case VarK:
    if (node->symbol->is_array)
    {
      char *buff = malloc(strlen(getName(node)) + 10);
      sprintf(buff, "-> array %s", getName(node));
      emitComment(buff);
      cgenArrayAddress(node);
      sprintf(buff, "<- array %s", getName(node));
      emitComment(buff);
      free(buff);
    }
    else
    {
      if (node->symbol->symbol_class == Global)
        emitRegAddr("lw", "$v0", getName(node), 0, NULL);
      else if (node->symbol->symbol_class == Local)
      {
        char *buff = malloc(strlen(getName(node)) + 20);
        sprintf(buff, "-> local variable %s", getName(node));
        emitComment(buff);
        emitRegReg("move", "$t0", "$fp");
        emitRegImm("addu", "$t0", node->symbol->memloc);
        emitRegAddr("lw", "$v0", NULL, 0, "$t0");
        sprintf(buff, "<- local variable %s", getName(node));
        emitComment(buff);
        free(buff);
      }
      else /* Parameter Variable */
      {
        char *buff = malloc(strlen(getName(node)) + 20);
        sprintf(buff, "-> parameter %s", getName(node));
        emitComment(buff);
        if (node->symbol->is_registered_argument)
          emitRegReg("move", "$v0", argumentRegisters[node->symbol->memloc]);
        else
        {
          emitRegReg("move", "$t0", "$fp");
          emitRegImm("addu", "$t0", node->symbol->memloc);
          emitRegAddr("lw", "$v0", NULL, 0, "$t0");
        }
        sprintf(buff, "<- parameter %s", getName(node));
        emitComment(buff);
        free(buff);
      }
    }
    break;
  case ArrK:
    cgenArrayAddress(node);
    cgenPush("$v0");
    cgenExp(node->child[0]);
    cgenPop("$t0"); /* array base: $t0, index: $v0 */
    emitRegRegImm("mul", "$v0", "$v0", WORD_SIZE);
    emitRegRegReg("addu", "$v0", "$v0", "$t0");
    emitRegAddr("lw", "$v0", NULL, 0, "$v0");
    break;
  case CallK:
    if (!strcmp("input", getName(node)))
    {
      /* Read integer from stdin to $v0 */
      emitComment("->call \'input\'");
      cgenPrintString("_inputStr");
      emitRegImm("li", "$v0", 5); /* syscall #5: read int */
      emitCode("syscall");
      emitComment("<-call \'input\'");
    }
    else if (!strcmp("output", getName(node)))
    {
      /* Print integer from $v0 to stdout */
      emitComment("->call \'output\'");
      cgenPush("$v0");
      cgenPush("$a0");
      cgenExp(node->child[0]); /* evaluate parameter */
      cgenPrintString("_outputStr");
      emitRegReg("move", "$a0", "$v0");
      emitRegImm("li", "$v0", 1); /* syscall #1: print int */
      emitCode("syscall");
      cgenPrintString("_newline");
      emitComment("<-call \'output\'");
      cgenPop("$a0");
      cgenPop("$v0");
    }
    else
    {
      /* Calling sequence */
      TreeNode *params;
      int i;
      emitComment("->call function");
      /* Save registered arguments of current function */
      for (i = 0; i < 4 && i < node->symbol->size; ++i)
        cgenPush(argumentRegisters[i]);
      /* Push arguments to stack */
      if (node->symbol->size > 4)
        emitRegRegImm("subu", "$sp", "$sp", WORD_SIZE * (node->symbol->size - 4));
      for (i = 0, params = node->child[0]; params; ++i, params = params->sibling)
      {
        cgenExp(params);
        if (i < 4) /* registered arguments */
          cgenPush("$v0");
        else /* stacked arguments */
          emitRegAddr("sw", "$v0", NULL, WORD_SIZE * (i - 4), "$sp");
      }
      for (i = 3; i >= 0; --i) /* Push registered arguments */
        if (i < node->symbol->size)
          cgenPop(argumentRegisters[i]);
      cgenPush("$fp");                        /* control link */
      emitRegReg("move", "$fp", "$sp");       /* new frame pointer */
      cgenPush("$ra");                        /* save return address */
      emitReg("jal", getName(node));          /* Jump to procedure */
      emitRegRegImm("subu", "$sp", "$fp", 4); /* pop arguments from stack */
      cgenPop("$ra");                         /* restore return address */
      cgenPop("$fp");                         /* restore frame pointer */
      if (node->symbol->size > 4)
        emitRegRegImm("addu", "$sp", "$sp", WORD_SIZE * (node->symbol->size - 4));
      for (i = 3; i >= 0; --i) /* Restore registered arguments */
        if (i < node->symbol->size)
          cgenPop(argumentRegisters[i]);
      emitComment("<-call function");
    }
    break;
  }
} /* cgenExp */

/* Procedure cgenString generates code
 * to print null-terminated ascii string
 * from the given label */
static void cgenPrintString(const char *symbol)
{
  cgenPush("$v0");
  cgenPush("$a0");
  emitRegImm("li", "$v0", 4); /* syscall #4: print string */
  emitRegAddr("la", "$a0", symbol, 0, NULL);
  emitCode("syscall");
  cgenPop("$a0");
  cgenPop("$v0");
} /* cgenString */

/* Procedure cgenOp generates code
 * for an operator and leaves result
 * at $t0 */
static void cgenOp(TreeNode *node)
{
  char *buff = malloc(15);
  sprintf(buff, "->operator %s", getOp(node->attr.op));
  emitComment(buff);
  cgenExp(node->child[0]); /* Operand 1 */
  cgenPush("$v0");
  cgenExp(node->child[1]); /* Operand 2 */
  emitRegReg("move", "$t1", "$v0");
  cgenPop("$t0"); /* $t0 op $t1 */
  switch (node->attr.op)
  {
  case PLUS:
    emitRegRegReg("add", "$v0", "$t0", "$t1");
    break;
  case MINUS:
    emitRegRegReg("sub", "$v0", "$t0", "$t1");
    break;
  case TIMES:
    emitRegRegReg("mul", "$v0", "$t0", "$t1");
    break;
  case OVER:
    emitReg("mflo", "$t3");
    emitRegReg("div", "$t0", "$t1");
    emitReg("mflo", "$v0");
    emitReg("mtlo", "$t3");
    break;
  case LT:
    emitRegRegReg("slt", "$v0", "$t0", "$t1");
    break;
  case LTE:
    emitRegRegReg("sle", "$v0", "$t0", "$t1");
    break;
  case GT:
    emitRegRegReg("sgt", "$v0", "$t0", "$t1");
    break;
  case GTE:
    emitRegRegReg("sge", "$v0", "$t0", "$t1");
    break;
  case EQ:
    emitRegRegReg("seq", "$v0", "$t0", "$t1");
    break;
  case NEQ:
    emitRegRegReg("sne", "$v0", "$t0", "$t1");
    break;
  }
  sprintf(buff, "<-operator %s", getOp(node->attr.op));
  emitComment(buff);
  free(buff);
} /* cgenOp */

/* Procedure cgenAssign generates code
 * to assign value of RHS
 * to memory indicated by LHS */
static void cgenAssign(TreeNode *node)
{
  TreeNode *LHS = node->child[0];
  emitComment("->Assign");
  /* Calculate address of LHS and save to $v0 */
  if (LHS->symbol->symbol_class == Global)
  { /* Global Variable/Array assignment */
    if (LHS->kind.exp == VarK)
      /* Global Variable */
      emitRegAddr("la", "$v0", getName(LHS), 0, NULL);
    else if (LHS->kind.exp == ArrK)
    { /* Global Array */
      /* evaluate array index */
      cgenExp(LHS->child[0]);
      emitRegRegImm("mul", "$v0", "$v0", WORD_SIZE);
      emitRegAddr("la", "$t0", getName(LHS), 0, NULL);
      emitRegRegReg("addu", "$v0", "$v0", "$t0");
    }
  }
  else /* Local Variable/Array assignment */
  {
    if ((LHS->symbol->treeNode->nodekind == DeclK && LHS->symbol->treeNode->kind.decl == VarDeclK) || (LHS->symbol->treeNode->nodekind == ParamK && LHS->symbol->treeNode->kind.param == VarParamK))
    { /* Local Variable */
      if (!node->child[0]->symbol->is_registered_argument)
      {
        emitRegReg("move", "$t0", "$fp");
        emitRegImm("addu", "$t0", node->child[0]->symbol->memloc);
        emitRegReg("move", "$v0", "$t0");
      }
    }
    else /* Local Array */
    {
      cgenArrayAddress(LHS); /* Array address in $v0  */
      cgenPush("$v0");

      cgenExp(LHS->child[0]); /* index in $v0 */
      cgenPop("$t0");
      emitRegRegImm("mul", "$v0", "$v0", WORD_SIZE);
      emitRegRegReg("addu", "$v0", "$t0", "$v0");
    }
  }
  cgenPush("$v0"); /* Save LHS address to stack*/
  cgenExp(node->child[1]);
  cgenPop("$t0");
  if (node->child[0]->symbol->is_registered_argument && !node->child[0]->symbol->is_array)
    emitRegReg("move", argumentRegisters[node->child[0]->symbol->memloc], "$v0");
  else
    emitRegReg("sw", "$v0", "0($t0)");
  emitComment("<-Assign");
} /* cgenAssign */

/* Procedure cgenCompound generates code
 * for compound statements */
static void cgenCompound(TreeNode *node)
{
  /* Skip declerations and run only statements */
  cgen(node->child[1]);
} /* cgenCompound */

/* Procedure cgen recursively generates code by
 * node traversal
 */
static void cgen(TreeNode *node)
{
  if (node != NULL)
  {
    switch (node->nodekind)
    {
    case StmtK:
      cgenStmt(node);
      break;
    case ExpK:
      cgenExp(node);
      break;
    case DeclK:
    case TypeK:
    case ParamK:
      break;
    }
    cgen(node->sibling);
  }
}

/* Procedure cgenPop generates code to
 * pop the top of stack to register */
static void cgenPop(const char *reg)
{
  emitRegAddr("lw", reg, NULL, 0, "$sp");
  emitRegRegImm("addu", "$sp", "$sp", WORD_SIZE);
}

/* Procedure cgenPush generates code to
 * push the register to the top of stack */
static void cgenPush(const char *reg)
{
  emitRegRegImm("subu", "$sp", "$sp", WORD_SIZE);
  emitRegAddr("sw", reg, NULL, 0, "$sp");
}

enum
{
  TEXT,
  DATA
} globalEmitMode = DATA;

/* Procedure cgenIOStrings generates
 * string data for input/output */
static void cgenIOStrings(void)
{
  emitComment("strings reserved for IO");
  emitCode(".data");
  emitCode("_inputStr:  .asciiz \"input: \"");
  emitCode("_outputStr: .asciiz \"output: \"");
  emitCode("_newline:   .asciiz \"\\n\"");
}

/* Procedure cgenGlobalVarDecl generates code for
 * global variable declaration
 */
const unsigned int ALIGN = 2; /* align memory to 2^(ALIGN) */
static void cgenGlobalVarDecl(const char *name, int size)
{
  char *buff = malloc(strlen(name) + 32);
  sprintf(buff, "->global variable \'%s\'", name);
  emitComment(buff);
  if (globalEmitMode != DATA)
  {
    globalEmitMode = DATA;
    emitCode(".data");
  }
  sprintf(buff, ".align %d", ALIGN);
  emitCode(buff);
  sprintf(buff, "%s: .space %d", name, size);
  emitCode(buff);
  sprintf(buff, "<-global variable \'%s\'", name);
  emitComment(buff);
  free(buff);
}

static void cgenFunDecl(TreeNode *node)
{
  /* Function Preamble */
  char *buff = malloc(strlen(getName(node)) + 37);
  sprintf(buff, "->function \'%s\'", getName(node));
  emitComment(buff);
  if (globalEmitMode != TEXT)
  {
    globalEmitMode = TEXT;
    emitCode(".text");
  }

  if (!strcmp(getName(node), "main"))
  {
    emitCode(".globl main");
    emitCode("main:");
    /* set frame pointer */
    emitRegReg("move", "$fp", "$sp");
  }
  else
  { /* only for non-main */
    returnLabel = getLabel();
    emitLabelStr(getName(node));
    emitComment("entry routine");
  }
  /* reserve space for local variables */
  emitRegRegImm("subu", "$sp", "$fp", -node->symbol->memloc);
  cgenCompound(node->child[2]); /* run the body code */
  if (strcmp(getName(node), "main"))
  { /* only for non-main */
    emitComment("exit routine");
    if (node->type == Integer)
      emitLabelNum(returnLabel);

    emitReg("jr", "$ra");
    sprintf(buff, "<-function \'%s\'", getName(node));
    emitComment(buff);
    returnLabel = -1;
  }
  free(buff);
}

/* Procedure cgenGlobal generates code for
 * the global scope
 */
static void cgenGlobal(TreeNode *node)
{
  if (node != NULL)
  {
    if (node->nodekind == DeclK)
    {
      if (strlen(node->symbol->treeNode->attr.name) == 1)
      {
        char *buff = malloc(3);
        addPtr(buff);
        buff[0] = '_';
        buff[1] = node->symbol->treeNode->attr.name[0];
        buff[2] = '\0';
        node->symbol->treeNode->attr.name = buff;
      }
      switch (node->kind.decl)
      {
      case VarDeclK:
        cgenGlobalVarDecl(getName(node), WORD_SIZE);
        break;
      case ArrDeclK:
        cgenGlobalVarDecl(getName(node), WORD_SIZE * node->child[1]->attr.val);
        break;
      case FunDeclK:
        cgenFunDecl(node);
        break;
      }
    }
    cgenGlobal(node->sibling);
  }
}

/* Procedure getLabel returns a new label number */
static int getLabel(void)
{
  static unsigned int labelN = 0;
  return labelN++;
}

/* Genetates code to calculate address
 * of given array and store it at $v0 */
static void cgenArrayAddress(TreeNode *node)
{
  if (node->symbol->symbol_class == Global)
    emitRegAddr("la", "$v0", getName(node), 0, NULL);
  else if (node->symbol->symbol_class == Local)
  {
    emitRegReg("move", "$v0", "$fp");
    emitRegImm("addu", "$v0", node->symbol->memloc);
  }
  else
  { /* Parameter */
    if (node->symbol->is_registered_argument)
      emitRegReg("move", "$v0", argumentRegisters[node->symbol->memloc]);
    else
    {
      emitRegReg("move", "$v0", "$fp");
      emitRegImm("addu", "$v0", node->symbol->memloc);
      emitRegAddr("lw", "$v0", NULL, 0, "$v0");
    }
  }
}

/**********************************************/
/* the primary function of the code generator */
/**********************************************/
/* Procedure codeGen generates code to a code
 * file by traversal of the syntax node. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */
void codeGen(TreeNode *syntaxTree, char *codefile)
{
  char *s = malloc(strlen(codefile) + 7);
  emitComment("C-Minus Compilation to SPIM Code");
  sprintf(s, "File: %s", codefile);
  emitComment(s);
  free(s);
  cgenIOStrings();
  cgenGlobal(syntaxTree);
  /* Exit routine. */
  emitComment("End of execution.");
  emitRegImm("li", "$v0", 10); /* syscall #10: exit */
  emitCode("syscall");
}
