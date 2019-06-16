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

/* prototypes for code generation functions */
static void cgen(TreeNode *tree);
static void cgenStmt( TreeNode * tree);
static void cgenExp( TreeNode * tree);
static void cgenDecl(TreeNode *tree);
static void cgenCompound(TreeNode *tree);
static void cgenPop(char *reg);

/* tmpOffset is the memory offset for temps
   It is decremented each time a temp is
   stored, and incremeted when loaded again
*/
// static int tmpOffset = 0;

/* Procedure cgenStmt generates code at a statement node */
static void cgenStmt( TreeNode * tree)
{
  switch(tree->kind.stmt) {
  case CompoundK:
    cgenCompound(tree);
    break;
  case SelectionK:
    /* NOT IMPLEMENTED */
    break;
  case IterationK:
    /* NOT IMPLEMENTED */
    break;
  case ReturnK:
    /* NOT IMPLEMENTED */
    break;
  }
} /* cgenStmt */

/* Procedure cgenExp generates code at an expression node
 * value of expression will be on top of stack */
static void cgenExp( TreeNode * tree)
{
  char *buff = malloc(65);
  // int loc;
  // TreeNode * p1, * p2;
  switch (tree->kind.exp) {
    case AssignK:
      /* NOT IMPLEMENTED */
      break;
    case OpK:
      /* NOT IMPLEMENTED */
      break;
    case ConstK :
      sprintf(buff, "->Const");
      emitComment(buff);
      emitRegImm("li", "$t0", tree->attr.val, "");
      emitRegAddr("sw", "$t0", 0, "$sp", "");
      emitRegRegImm("subu", "$sp", "$sp", 4, "");
      sprintf(buff, "<-Const");
      emitComment(buff);

      break;
    case VarK:
      /* NOT IMPLEMENTED */
      break;
    case ArrK:
      /* NOT IMPLEMENTED */
      break;
    case CallK:
      if(!strcmp("input", tree->attr.name))
      {
        /* Read integer from stdin to $v0 */
        /* NOT IMPLEMENTED */
      }
      else if(!strcmp("output", tree->attr.name))
      {
        sprintf(buff, "->output call");
        emitComment(buff);
        cgenExp(tree->child[0]);
        cgenPop("$a0");
        emitRegImm("li", "$v0", 1, "");
        emitCode("syscall", "");
        sprintf(buff, "<-output call");
        emitComment(buff);
      }
      else
      {
        /* General function call */
        /* NOT IMPLEMENTED */
      }
      break;
  }
  free(buff);
} /* cgenExp */

/* Procedue cgenDecl generates code
 * for local variable declarations */
static void cgenDecl(TreeNode *tree)
{
  /* NOT IMPLEMENTED */
} /* cgenDecl */

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
        cgenDecl(tree);
        break;
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
  emitRegRegImm("addu", "$sp", "$sp", 4, "");
  emitRegAddr("lw", reg, 0, "$sp", "");
}

enum { NONE, TEXT, DATA } globalEmitMode = NONE;

/* Procedure cgenGlobalVarDecl generates code for
 * global variable declaration
 */
const unsigned int ALIGN = 2; /* align memory to 2^(ALIGN) */
static void cgenGlobalVarDecl(char *name, int size)
{
  char *buff = malloc(strlen(name) + 32);
  sprintf(buff, "global variable %s", name);
  emitComment(buff);
  if(globalEmitMode != DATA)
  {
    globalEmitMode = DATA;
    emitCode(".data", "");
  }
  sprintf(buff, ".align %d", ALIGN);
  emitCode(buff, "align on a word boundary");
  sprintf(buff, "_%s: .space %d", name, size);
  emitCode(buff, "reserve space for variable");
  free(buff);
  emitBlank();
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
    emitCode(".text", "");
  }

  if(!strcmp(node->attr.name, "main"))
  {
    emitCode(".globl main", "");
    emitCode("main:", "");
  }
  else
  {
    sprintf(buff, "_%s:", node->attr.name);
    emitCode(buff, "");
  }
  cgenCompound(node->child[2]);
  sprintf(buff, "end of function \'%s\'", node->attr.name);
  emitComment(buff);
  free(buff);
  emitBlank();
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
{  char * s = malloc(strlen(codefile)+7);
   strcpy(s,"File: ");
   strcat(s,codefile);
   emitComment("C-Minus Compilation to SPIM Code");
   emitComment(s);
   free(s);
   cgenGlobal(syntaxTree);
   /* Exit routine. */
   emitComment("End of execution.");
   emitRegImm("li", "$v0", 10, "");
   emitCode("syscall", "");
}