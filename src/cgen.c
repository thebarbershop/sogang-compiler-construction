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
static void cgenPop(char *reg);
static void cgenPush(char *reg); 
static void cgenString(char *label);
static void cgenLabel(char *label);
static char* getLabel(void);

static char *returnLabel; /* return label used in a function */

/* Functions to generate decorated strings  */
/* Generate string "symbol±imm(register)" */
static char *addrSymbolImmReg(const char *symbol, char sign, int imm, const char *reg)
{
  char* buff = malloc(strlen(symbol)+strlen(reg)+14);
  sprintf(buff, "_%s%c%d(%s)", symbol, sign, imm, reg);
  addPtr(buff);
  return buff;
}

static char *underbar(const char *label)
{
  char* buff = malloc(strlen(label)+2);
  sprintf(buff, "_%s", label);
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
    char* followingLabel = getLabel();
    emitComment("->selection");
    cgenExp(node->child[0]);
    if(node->child[2])
    { /* Has else statement */
      char* elseLabel = getLabel();
      emitRegReg("beqz", "$t0", elseLabel);
      cgen(node->child[1]);
      emitReg("b", followingLabel);
      cgenLabel(elseLabel);
      cgen(node->child[2]);
    }
    else
    { /* No else statement */
      emitRegReg("beqz", "$t0", followingLabel);
      cgen(node->child[1]);
    }
    cgenLabel(followingLabel);
    emitComment("<-selection");
    break;
  }
  case IterationK:
  {
    char* conditionLabel = getLabel();
    char* followingLabel = getLabel();
    emitComment("->iteration");
    cgenLabel(conditionLabel);
    cgenExp(node->child[0]);
    emitRegReg("beqz", "$t0", followingLabel);
    cgen(node->child[1]);
    emitReg("b", conditionLabel);
    cgenLabel(followingLabel);
    emitComment("<-iteration");
    break;
  }
  case ReturnK:
    cgenExp(node->child[0]);
    emitRegReg("move", "$v0", "$t0");
    emitReg("j", returnLabel);
    break;
  }
} /* cgenStmt */

/* Procedure cgenExp generates code at an expression node
 * Value of expression will be in $t0 */
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
      emitRegImm("li", "$t0", node->attr.val);
      emitComment("<-Const");
      break;
    case VarK:
      if(node->symbol->symbol_class == Global)
        emitRegReg("lw", "$t0", underbar(node->attr.name));
      else if(node->symbol->symbol_class == Local)
      {
        char *buff = malloc(strlen(node->attr.name)+20);
        int offset = node->symbol->memloc;
        if(returnLabel && offset<0) offset -= 8;
        sprintf(buff, "-> local variable %s", node->attr.name);
        emitComment(buff);
        emitRegReg("move", "$t1", "$fp");
        emitRegImm("addu", "$t1", offset);
        emitRegAddr("lw", "$t0", 0, "$t1");
        sprintf(buff, "<- local variable %s", node->attr.name);
        emitComment(buff);
        free(buff);
      }
      break;
    case ArrK:
      if(node->symbol->symbol_class == Global)
      {
        cgenExp(node->child[0]);
        emitRegRegImm("mul", "$t1", "$t0", WORD_SIZE);
        emitRegReg("lw", "$t0", addrSymbolImmReg(underbar(node->attr.name), '+', 0, "$t1"));
      }
      else if(node->symbol->symbol_class == Local)
      {
        /* TODO: implement local array reference */
      }
      break;
    case CallK:
      if(!strcmp("input", node->attr.name))
      {
        /* Read integer from stdin to $v0 */
        emitComment("->call \'input\'");
        cgenString("_inputStr");
        emitRegImm("li", "$v0", 5); /* syscall #5: read int */
        emitCode("syscall");
        emitRegReg("move", "$t0", "$v0");
        emitComment("<-call \'input\'");
      }
      else if(!strcmp("output", node->attr.name))
      {
        /* Print integer from $a0 to stdout */
        emitComment("->call \'output\'");
        cgenString("_outputStr");
        cgenExp(node->child[0]); /* evaluate parameter */
        emitRegReg("move", "$a0", "$t0");
        emitRegImm("li", "$v0", 1); /* syscall #1: print int */
        emitCode("syscall");
        cgenString("_newline");
        emitComment("<-call \'output\'");
      }
      else
      {
        TreeNode * params;
        /* General function call */
        emitComment("->call function");
        /* Push arguments to stack */
        for(params = node->child[0]; params; params = params->sibling)
        {
          cgenExp(params);
          cgenPush("$t0");
        }
        emitReg("jal", underbar(node->attr.name));
        emitRegReg("move", "$t0", "$v0");
        /* pop arguments from stack */
        emitRegRegImm("addu", "$sp", "$sp", WORD_SIZE*(node->symbol->size+1));
        emitComment("<-call function");
      }
      break;
  }
} /* cgenExp */

/* Procedure cgenString generates code
 * to print null-terminated ascii string
 * from the given label */
static void cgenString(char *label)
{
  emitRegImm("li", "$v0", 4); /* syscall #4: print string */
  emitRegReg("la", "$a0", label);
  emitCode("syscall");
} /* cgenString */

/* Procedure cgenOp generates code
 * for an operator and leaves result
 * at $t0 */
static void cgenOp(TreeNode *node)
{
  char* buff = malloc(15);
  sprintf(buff, "->operator %s", getOp(node->attr.op));
  emitComment(buff);
  cgenExp(node->child[0]);
  cgenPush("$t0");
  cgenExp(node->child[1]);
  emitRegReg("move", "$t1", "$t0");
  cgenPop("$t0");
  switch(node->attr.op) {
  case PLUS:
    emitRegRegReg("add", "$t0", "$t0", "$t1");
    break;
  case MINUS:
    emitRegRegReg("sub", "$t0", "$t0", "$t1");
    break;
  case TIMES:
    emitRegRegReg("mul", "$t0", "$t0", "$t1");
    break;
  case OVER:
    emitReg("mflo", "$t3");
    emitRegReg("div", "$t0", "$t1");
    emitReg("mflo", "$t0");
    emitReg("mtlo", "$t3");
    break;
  case LT:
    emitRegRegReg("slt", "$t0", "$t0", "$t1");
    break;
  case LTE:
    emitRegRegReg("slte", "$t0", "$t0", "$t1");
    break;
  case GT:
    emitRegRegReg("sgt", "$t0", "$t0", "$t1");
    break;
  case GTE:
    emitRegRegReg("sgte", "$t0", "$t0", "$t1");
    break;
  case EQ:
    emitRegRegReg("seq", "$t0", "$t0", "$t1");
    break;
  case NEQ:
    emitRegRegReg("sne", "$t0", "$t0", "$t1");
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
  cgenExp(node->child[1]); /* save rhs to top of stack */
  cgenPush("$t0");
  if(node->child[0]->symbol->symbol_class == Global)
  { /* generate address computation string */
    if(node->child[0]->kind.exp == VarK) {
      addressLHS = underbar(node->child[0]->attr.name);
    }
    else if(node->child[0]->kind.exp == ArrK)
    {
      /* evaluate array index */
      cgenExp(node->child[0]->child[0]);
      /* offset is WORD_SIZE*index */
      emitRegRegImm("mul", "$t0", "$t0", WORD_SIZE);
      /* addressing mode: symbol+0(register) */
      addressLHS = addrSymbolImmReg(node->child[0]->attr.name, '+', 0, "$t0");
    }
  }
  else if(node->child[0]->symbol->symbol_class == Local)
  {
    if(node->child[0]->symbol->is_array)
    {
      /* $fp - memloc(4*index) */
      cgenExp(node->child[0]->child[0]);
      addressLHS = addrSymbolImmReg("$fp", '+', 0, "$t0");
    }
    else
    {
      /* $fp + memloc */
      int offset = node->child[0]->symbol->memloc;
      if(returnLabel && offset < 0) offset -= 8;
      emitRegReg("move", "$t2", "$fp");
      emitRegImm("addu", "$t2", offset);
      addressLHS = parentheses("$t2");
    }
  }
  cgenPop("$t1"); /* load rhs from top of stack */
  emitRegReg("sw", "$t1", addressLHS);
  emitComment("<-Assign");
} /* cgenAssign */

/* Procedure cgenCompound generates code
 * for compound statements */
static void cgenCompound(TreeNode *node)
{
  cgen(node->child[0]);
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
static void cgenPop(char *reg)
{
  emitRegRegImm("addu", "$sp", "$sp", WORD_SIZE);
  emitRegAddr("lw", reg, 0, "$sp");
}

/* Procedure cgenPush generates code to
 * push the register to the top of stack */
static void cgenPush(char *reg)
{
  emitRegAddr("sw", reg, 0, "$sp");
  emitRegRegImm("subu", "$sp", "$sp", WORD_SIZE);
}

/* Procedure cgenLabel generates code to
 * define a label */
static void cgenLabel(char *label)
{
  char *buff = malloc(strlen(label)+2);
  sprintf(buff, "%s:", label);
  emitCode(buff);
  free(buff);
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
static void cgenGlobalVarDecl(char *name, int size)
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
  sprintf(buff, "_%s: .space %d", name, size);
  emitCode(buff);
  sprintf(buff, "<-global variable \'%s\'", name);
  emitComment(buff);
  free(buff);
}

static void cgenFunDecl(TreeNode *node)
{
  /* Function Preamble */
  char *buff = malloc(strlen(node->attr.name) + 37);
  sprintf(buff, "->function \'%s\'", node->attr.name);
  emitComment(buff);
  if(globalEmitMode != TEXT)
  {
    globalEmitMode = TEXT;
    emitCode(".text");
  }

  if(!strcmp(node->attr.name, "main"))
  {
    emitCode(".globl main");
    emitCode("main:");
    /* set frame pointer */
    emitRegReg("move", "$fp", "$sp");
  }
  else
  {
    returnLabel = getLabel();
    sprintf(buff, "_%s:", node->attr.name);
    emitCode(buff);
    emitComment("entry routine");
    cgenPush("$ra"); /* save return address */
    cgenPush("$fp"); /* save frame pointer */
    /* set frame pointer of new function */
    emitRegRegImm("addu", "$fp", "$sp", 3*WORD_SIZE);
  }
  /* reserve space for local variables */
  emitRegRegImm("subu", "$sp", "$sp", -node->symbol->memloc);
  cgenCompound(node->child[2]);
  if(strcmp(node->attr.name, "main")){ /* only for non-main */
    emitComment("exit routine");
    if(node->type == Integer)
      cgenLabel(returnLabel);
    
    emitRegRegImm("subu", "$sp", "$fp", 3*WORD_SIZE);
    cgenPop("$fp"); /* restore frame pointer */
    cgenPop("$ra"); /* restore return address */
    emitReg("jr", "$ra");
    sprintf(buff, "<-function \'%s\'", node->attr.name);
    emitComment(buff);
    returnLabel = NULL;
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
      switch(node->kind.decl) {
        case VarDeclK:
          cgenGlobalVarDecl(node->attr.name, WORD_SIZE);
          break;
        case ArrDeclK:
          cgenGlobalVarDecl(node->attr.name, WORD_SIZE*node->child[1]->attr.val);
          break;
        case FunDeclK:
          cgenFunDecl(node);
          break;
      }
    }
    cgenGlobal(node->sibling);
  }
}


static char* getLabel(void)
{
  static unsigned int labelN = 0;

  char* buff;
  int tmp, lengthN;
  for(lengthN = 0, tmp = labelN; tmp; ++lengthN)
    tmp /= 10;
  buff = malloc(lengthN+3);
  sprintf(buff, "L%d", labelN++);
  addPtr(buff);
  return buff;
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
  cgenIOStrings();
  cgenGlobal(syntaxTree);
  /* Exit routine. */
  emitComment("End of execution.");
  emitRegImm("li", "$v0", 10); /* syscall #10: exit */
  emitCode("syscall");
  free(s);
}