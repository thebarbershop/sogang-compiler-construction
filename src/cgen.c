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
static void cgenStmt( TreeNode * node);
static void cgenExp( TreeNode * node);
static void cgenOp(TreeNode *node);
static void cgenAssign(TreeNode *node);
static void cgenCompound(TreeNode *node);
static void cgenPop(const char *reg);
static void cgenPush(const char *reg); 
static void cgenPrintString(const char* symbol);
static int getLabel(void);
static void cgenArrayAddress(TreeNode *node);

static int returnLabel; /* return label used in a function */

/* Functions to generate decorated strings  */
/* Generate string "symbol±imm(register)" */
static char *addrSymbolImmReg(const char *symbol, char sign, int imm, const char *reg)
{
  char* buff = malloc(strlen(symbol)+strlen(reg)+14);
  sprintf(buff, "%s%c%d(%s)", symbol, sign, imm, reg);
  addPtr(buff);
  return buff;
}

static char *parentheses(const char *reg) {
  char* buff = malloc(strlen(reg)+3);
  sprintf(buff, "(%s)", reg);
  addPtr(buff);
  return buff;
}

/* Procedure cgenStmt generates code at a statement node */
static void cgenStmt( TreeNode * node)
{
  switch(node->kind.stmt) {
  case CompoundK:
    cgenCompound(node);
    break;
  case SelectionK:
  {
    int followingLabel = getLabel();
    emitComment("->selection");
    cgenExp(node->child[0]);
    if(node->child[2])
    { /* Has else statement */
      int elseLabel = getLabel();
      emitRegLabel("beqz", "$v0", elseLabel);
      cgen(node->child[1]);
      emitLabel("b", followingLabel);
      emitLabelDecl(elseLabel);
      cgen(node->child[2]);
    }
    else
    { /* No else statement */
      emitRegLabel("beqz", "$v0", followingLabel);
      cgen(node->child[1]);
    }
    emitLabelDecl(followingLabel);
    emitComment("<-selection");
    break;
  }
  case IterationK:
  {
    int conditionLabel = getLabel();
    int followingLabel = getLabel();
    emitComment("->iteration");
    emitLabelDecl(conditionLabel);
    cgenExp(node->child[0]);
    emitRegLabel("beqz", "$v0", followingLabel);
    cgen(node->child[1]);
    emitLabel("b", conditionLabel);
    emitLabelDecl(followingLabel);
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
static void cgenExp( TreeNode * node)
{
  switch (node->kind.exp) {
    case AssignK:
      cgenAssign(node);
      break;
    case OpK:
      cgenOp(node);
      break;
    case ConstK :
      emitComment("->Const");
      emitRegImm("li", "$v0", node->attr.val);
      emitComment("<-Const");
      break;
    case VarK:
      if(node->symbol->is_array)
      {
        char *buff = malloc(strlen(node->symbol->treeNode->attr.name)+10);
        sprintf(buff, "-> array %s", node->symbol->treeNode->attr.name);
        emitComment(buff);
        cgenArrayAddress(node);
        sprintf(buff, "<- array %s", node->symbol->treeNode->attr.name);
        emitComment(buff);
        free(buff);
      }
      else
      {
        if(node->symbol->symbol_class == Global)
          emitRegSymbol("lw", "$v0", node->symbol->treeNode->attr.name);
        else
        {
          char *buff = malloc(strlen(node->symbol->treeNode->attr.name)+20);
          int offset = node->symbol->memloc;
          if(returnLabel > 0 && offset<0) offset -= 8;
          sprintf(buff, "-> local variable %s", node->symbol->treeNode->attr.name);
          emitComment(buff);
          emitRegReg("move", "$t0", "$fp");
          emitRegImm("addu", "$t0", offset);
          emitRegAddr("lw", "$v0", 0, "$t0");
          sprintf(buff, "<- local variable %s", node->symbol->treeNode->attr.name);
          emitComment(buff);
          free(buff);
        }
      }
      break;
    case ArrK:
      cgenArrayAddress(node);
      cgenPush("$v0");
      cgenExp(node->child[0]);
      cgenPop("$t1"); /* array base: $t1, index: $v0 */
      emitRegRegImm("mul", "$v0", "$v0", WORD_SIZE);
      emitRegRegReg("addu", "$v0", "$v0", "$t1");
      emitRegReg("lw", "$v0", parentheses("$v0"));
      break;
    case CallK:
      if(!strcmp("input", node->symbol->treeNode->attr.name))
      {
        /* Read integer from stdin to $v0 */
        emitComment("->call \'input\'");
        cgenPrintString("_inputStr");
        emitRegImm("li", "$v0", 5); /* syscall #5: read int */
        emitCode("syscall");
        emitComment("<-call \'input\'");
      }
      else if(!strcmp("output", node->symbol->treeNode->attr.name))
      {
        /* Print integer from $v0 to stdout */
        emitComment("->call \'output\'");
        cgenExp(node->child[0]); /* evaluate parameter */
        cgenPrintString("_outputStr");
        emitRegReg("move", "$a0", "$v0");
        emitRegImm("li", "$v0", 1); /* syscall #1: print int */
        emitCode("syscall");
        cgenPrintString("_newline");
        emitComment("<-call \'output\'");
      }
      else
      {
        TreeNode * params;
        int i;
        /* General function call */
        emitComment("->call function");
        /* Push arguments to stack */
        emitRegRegImm("subu", "$sp", "$sp", WORD_SIZE*(node->symbol->size));
        for(i = 0, params = node->child[0]; params; ++i, params = params->sibling)
        {
          cgenExp(params);
          emitRegAddr("sw", "$v0", WORD_SIZE*i, "$sp");
        }
        fprintf(stderr, "\n");
        emitReg("jal", node->symbol->treeNode->attr.name);
        /* pop arguments from stack */
        emitRegRegImm("addu", "$sp", "$sp", WORD_SIZE*(node->symbol->size));
        emitComment("<-call function");
      }
      break;
  }
} /* cgenExp */

/* Procedure cgenString generates code
 * to print null-terminated ascii string
 * from the given label */
static void cgenPrintString(const char* symbol)
{
  emitRegReg("move", "$t0", "$v0");
  emitRegImm("li", "$v0", 4); /* syscall #4: print string */
  emitRegSymbol("la", "$a0", symbol);
  emitCode("syscall");
  emitRegReg("move", "$v0", "$t0");
} /* cgenString */

/* Procedure cgenOp generates code
 * for an operator and leaves result
 * at $t0 */
static void cgenOp(TreeNode *node)
{
  char* buff = malloc(15);
  sprintf(buff, "->operator %s", getOp(node->attr.op));
  emitComment(buff);
  cgenExp(node->child[0]); /* Operand 1 */
  cgenPush("$v0");
  cgenExp(node->child[1]); /* Operand 2 */
  emitRegReg("move", "$t1", "$v0");
  cgenPop("$t0");          /* $t0 op $t1 */
  switch(node->attr.op) {
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
  char *addressLHS;
  emitComment("->Assign");
  cgenExp(node->child[1]); /* save RHS to top of stack */
  cgenPush("$v0");
  if(node->child[0]->symbol->symbol_class == Global)
  { /* generate address computation string */
    if(node->child[0]->kind.exp == VarK) {
      addressLHS = node->child[0]->symbol->treeNode->attr.name;
    }
    else if(node->child[0]->kind.exp == ArrK)
    {
      /* evaluate array index */
      cgenExp(node->child[0]->child[0]);
      /* offset is WORD_SIZE*index */
      emitRegRegImm("mul", "$v0", "$v0", WORD_SIZE);
      /* addressing mode: symbol+0(register) */
      addressLHS = addrSymbolImmReg(node->child[0]->symbol->treeNode->attr.name, '+', 0, "$v0");
    }
  }
  else
  {
    if(node->child[0]->symbol->is_array)
    {
      cgenArrayAddress(node->child[0]);
      emitRegReg("move", "$t2", "$v0");

      /* get address of indexed element */
      cgenExp(node->child[0]->child[0]); /* index in $v0 */
      emitRegRegImm("mul", "$v0", "$v0", WORD_SIZE);
      emitRegRegReg("addu", "$t2", "$t2", "$v0");
      addressLHS = parentheses("$t2");
    }
    else
    {
      /* $fp + memloc */
      int offset = node->child[0]->symbol->memloc;
      if(returnLabel > 0 && offset < 0) offset -= 8;
      emitRegReg("move", "$t2", "$fp");
      emitRegImm("addu", "$t2", offset);
      addressLHS = parentheses("$t2");
    }
  }
  cgenPop("$t0"); /* load RHS from top of stack */
  emitRegReg("sw", "$t0", addressLHS);
  emitRegReg("move", "$v0", "$t0");
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
static void cgen(TreeNode * node)
{
  if (node != NULL)
  {
    switch (node->nodekind) {
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
  emitRegAddr("lw", reg, 0, "$sp");
  emitRegRegImm("addu", "$sp", "$sp", WORD_SIZE);
}

/* Procedure cgenPush generates code to
 * push the register to the top of stack */
static void cgenPush(const char *reg)
{
  emitRegRegImm("subu", "$sp", "$sp", WORD_SIZE);
  emitRegAddr("sw", reg, 0, "$sp");
}

enum { TEXT, DATA } globalEmitMode = DATA;

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
  if(globalEmitMode != DATA)
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
  char *buff = malloc(strlen(node->symbol->treeNode->attr.name) + 37);
  sprintf(buff, "->function \'%s\'", node->symbol->treeNode->attr.name);
  emitComment(buff);
  if(globalEmitMode != TEXT)
  {
    globalEmitMode = TEXT;
    emitCode(".text");
  }

  if(!strcmp(node->symbol->treeNode->attr.name, "_main"))
  {
    emitCode(".globl main");
    emitCode("main:");
    /* set frame pointer */
    emitRegReg("move", "$fp", "$sp");
  }
  else
  { /* only for non-main */
    returnLabel = getLabel();
    emitSymbolDecl(node->symbol->treeNode->attr.name);
    emitComment("entry routine");
    cgenPush("$ra"); /* save return address */
    cgenPush("$fp"); /* save frame pointer */
    /* set frame pointer of new function */
    emitRegRegImm("addu", "$fp", "$sp", 2*WORD_SIZE);
  }
  /* reserve space for local variables */
  emitRegRegImm("subu", "$sp", "$sp", -node->symbol->memloc-WORD_SIZE);
  cgenCompound(node->child[2]); /* run the body code */
  if(strcmp(node->symbol->treeNode->attr.name, "main"))
  { /* only for non-main */
    emitComment("exit routine");
    if(node->type == Integer)
      emitLabelDecl(returnLabel);
    
    emitRegRegImm("subu", "$sp", "$fp", 2*WORD_SIZE);
    cgenPop("$fp"); /* restore frame pointer */
    cgenPop("$ra"); /* restore return address */
    emitReg("jr", "$ra");
    sprintf(buff, "<-function \'%s\'", node->symbol->treeNode->attr.name);
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
    if(node->nodekind == DeclK) {
      if(strlen(node->symbol->treeNode->attr.name) == 1) {
        char buff[3];
        buff[0] = '_';
        buff[1] = node->symbol->treeNode->attr.name[0];
        buff[2] = '\0';
        node->symbol->treeNode->attr.name = buff;
        node->symbol->treeNode->attr.name = buff;
      }
      switch(node->kind.decl) {
        case VarDeclK:
          cgenGlobalVarDecl(node->symbol->treeNode->attr.name, WORD_SIZE);
          break;
        case ArrDeclK:
          cgenGlobalVarDecl(node->symbol->treeNode->attr.name, WORD_SIZE*node->child[1]->attr.val);
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

/* Procedure cgenArrayAddress generates
 * code to calculate address of given array
 * and store it at $v0 */
static void cgenArrayAddress(TreeNode *node)
{
  int offset = node->symbol->memloc;
  if(returnLabel > 0 && offset<0) offset -= 8;
  if(node->symbol->symbol_class == Global)
    emitRegSymbol("la", "$v0", node->symbol->treeNode->attr.name);
  else if(node->symbol->symbol_class == Local) {
    emitRegReg("move", "$v0", "$fp");
    emitRegImm("addu", "$v0", offset);
  }
  else { /* Parameter */
    emitRegReg("move", "$v0", "$fp");
    emitRegImm("addu", "$v0", offset);
    emitRegAddr("lw", "$v0", 0, "$v0");
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
void codeGen(TreeNode * syntaxTree, char * codefile)
{
  char * s = malloc(strlen(codefile)+7);
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