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
static void cgen(TreeNode *tree);
static void cgenStmt( TreeNode * tree);
static void cgenExp( TreeNode * tree);
static void cgenOp(TreeNode *tree);
static void cgenAssign(TreeNode *tree);
static void cgenCompound(TreeNode *tree);
static void cgenPop(char *reg);
static void cgenPush(char *reg);
static void cgenString(char *label);
static void cgenLabel(char *label);
static char* getLabel(void);

/* Procedure cgenStmt generates code at a statement node */
static void cgenStmt( TreeNode * tree)
{
  switch(tree->kind.stmt) {
  case CompoundK:
    cgenCompound(tree);
    break;
  case SelectionK:
  {
    char* followingLabel = getLabel();
    emitComment("->selection");
    cgenExp(tree->child[0]);
    if(tree->child[2])
    { /* Has else statement */
      char* elseLabel = getLabel();
      emitRegReg("beqz", "$t0", elseLabel);
      cgen(tree->child[1]);
      emitReg("b", followingLabel);
      cgenLabel(elseLabel);
      cgen(tree->child[2]);
    }
    else
    { /* No else statement */
      emitRegReg("beqz", "$t0", followingLabel);
      cgen(tree->child[1]);
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
    cgenExp(tree->child[0]);
    emitRegReg("beqz", "$t0", followingLabel);
    cgen(tree->child[1]);
    emitReg("b", conditionLabel);
    cgenLabel(followingLabel);
    emitComment("<-iteration");
    break;
  }
  case ReturnK:
    /* NOT IMPLEMENTED */
    break;
  }
} /* cgenStmt */

/* Procedure cgenExp generates code at an expression node
 * Value of expression will be in $t0 */
static void cgenExp( TreeNode * tree)
{
  switch (tree->kind.exp) {
    case AssignK:
      cgenAssign(tree);
      break;
    case OpK:
      cgenOp(tree);
      break;
    case ConstK :
      emitComment("->Const");
      emitRegImm("li", "$t0", tree->attr.val);
      emitComment("<-Const");
      break;
    case VarK:
      if(tree->symbol->symbol_class == GlobalVariable)
        emitRegReg("lw", "$t0", tree->symbol->name);
      else if(tree->symbol->symbol_class == LocalVariable)
      {
        /* NOT IMPLEMENTED */
      }
      break;
    case ArrK:
      if(tree->symbol->symbol_class == GlobalVariable)
      {
        cgenExp(tree->child[0]);
        emitRegRegImm("mul", "$t1", "$t0", WORD_SIZE);
        emitRegReg("lw", "$t0", addrSymbolImmReg(tree->symbol->name, '+', 0, "$t1"));
      }
      else if(tree->symbol->symbol_class == LocalVariable)
      {
        /* NOT IMPLEMENTED */
      }
      break;
    case CallK:
      if(!strcmp("input", tree->attr.name))
      {
        /* Read integer from stdin to $v0 */
        emitComment("->input call");
        cgenString("_inputStr");
        /* NOT IMPLEMENTED */
        emitComment("<-input call");
      }
      else if(!strcmp("output", tree->attr.name))
      {
        emitComment("->output call");
        cgenString("_outputStr");
        cgenExp(tree->child[0]); /* evaluate parameter */
        emitRegReg("move", "$a0", "$t0");
        emitRegImm("li", "$v0", 1); /* syscall #1: print int */
        emitCode("syscall");
        cgenString("_newline");
        emitComment("<-output call");
      }
      else
      {
        /* General function call */
        /* NOT IMPLEMENTED */
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
static void cgenOp(TreeNode *tree)
{
  char* buff = malloc(15);
  sprintf(buff, "->operator %s", getOp(tree->attr.op));
  emitComment(buff);
  cgenExp(tree->child[0]);
  cgenPush("$t0");
  cgenExp(tree->child[1]);
  emitRegReg("move", "$t1", "$t0");
  cgenPop("$t0");
  switch(tree->attr.op) {
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
  sprintf(buff, "<-operator %s", getOp(tree->attr.op));
  emitComment(buff);
  free(buff);
} /* cgenOp */


/* Procedure cgenAssign generates code
 * to assign value of RHS
 * to memory indicated by LHS */
static void cgenAssign(TreeNode *tree)
{
  char *addressLHS;
  emitComment("->Assign");
  cgenExp(tree->child[1]); /* save rhs to top of stack */
  cgenPush("$t0");
  if(tree->child[0]->symbol->symbol_class == GlobalVariable)
  { /* generate address computation string */
    if(tree->child[0]->kind.exp == VarK) {
      addressLHS = tree->child[0]->symbol->name;
    }
    else if(tree->child[0]->kind.exp == ArrK)
    {
      /* evaluate array index */
      cgenExp(tree->child[0]->child[0]);
      /* offset is WORD_SIZE*index */
      emitRegRegImm("mul", "$t0", "$t0", WORD_SIZE);
      addressLHS = malloc(strlen(tree->child[0]->symbol->name)+7);
      addPtr(addressLHS);
      /* addressing mode: symbol+(register) */
      sprintf(addressLHS, "%s+0($t0)", tree->child[0]->symbol->name);
    }
  }
  else if(tree->child[0]->symbol->symbol_class == LocalVariable)
  {
    /* NOT IMPLEMENTED */
    addressLHS = "";
  }
  cgenPop("$t1"); /* load rhs from top of stack */
  emitRegReg("sw", "$t1", addressLHS);
  emitComment("<-Assign");
} /* cgenAssign */

/* Procedure cgenCompound generates code
 * for compound statements */
static void cgenCompound(TreeNode *tree)
{
  cgen(tree->child[0]);
  cgen(tree->child[1]);
} /* cgenCompound */

/* Procedure cgen recursively generates code by
 * tree traversal
 */
static void cgen(TreeNode * tree)
{
  if (tree != NULL)
  {
    switch (tree->nodekind) {
      case StmtK:
        cgenStmt(tree);
        break;
      case ExpK:
        cgenExp(tree);
        break;
      case DeclK:
      case TypeK:
      case ParamK:
        break;
    }
    cgen(tree->sibling);
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
  sprintf(buff, "global variable \'%s\'", name);
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
  free(buff);
}

static void cgenFunDecl(TreeNode *node)
{
  /* Function Preamble */
  char *buff = malloc(strlen(node->attr.name) + 37);
  sprintf(buff, "procedure for function \'%s\'", node->attr.name);
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
  }
  else
  {
    sprintf(buff, "%s:", node->attr.name);
    emitCode(buff);
  }
  cgenCompound(node->child[2]);
  sprintf(buff, "end of function \'%s\'", node->attr.name);
  emitComment(buff);
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
    else {
      fprintf(listing, "something wrong in global nodes\n");
      fprintf(listing, "lineno: %d\n, nodekind: %d\n", node->lineno, node->nodekind);
      Error = TRUE;
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
 * file by traversal of the syntax tree. The
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