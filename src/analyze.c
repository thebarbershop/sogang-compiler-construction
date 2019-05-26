/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the C- compiler                              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

static void typeError(TreeNode *t, const char *message)
{
  fprintf(listing, "Type error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
}

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse(TreeNode *t,
                     void (*preProc)(TreeNode *),
                     void (*postProc)(TreeNode *))
{
  if (t != NULL)
  {
    preProc(t);
    {
      int i;
      for (i = 0; i < MAXCHILDREN; i++) {
        traverse(t->child[i], preProc, postProc);
      }
    }
    postProc(t);
    traverse(t->sibling, preProc, postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode *t)
{
  if (t == NULL)
    return;
  else
    return;
}

static int flag_functionDeclared = 0;
/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode(TreeNode *t)
{
  while(t) {
    switch (t->nodekind)
    {
    case StmtK:
      switch(t->kind.stmt)
      {
        case CompoundK:
        {
          int scope_incremented = 0;
          if(!flag_functionDeclared)
          {
            incrementScope();
            scope_incremented = 1;
          }
          else
            flag_functionDeclared = 0;
          insertNode(t->child[0]);
          insertNode(t->child[1]);
          if(TraceAnalyze)
            printSymTab(listing);
          if(scope_incremented)
            decrementScope();
          break;
        }
        case SelectionK:
          insertNode(t->child[0]);
          insertNode(t->child[1]);
          insertNode(t->child[2]);
          break;
        case IterationK:
          insertNode(t->child[0]);
          insertNode(t->child[1]);
          break;
        case ReturnK:
          insertNode(t->child[0]);
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      {
      case AssignK:
        insertNode(t->child[1]);
        insertNode(t->child[0]);
        break;
      case OpK:
        insertNode(t->child[0]);
        insertNode(t->child[1]);
        break;
      case ConstK:
        break;
      case VarK:
        referenceSymbol(t);
        break;
      case ArrK:
        referenceSymbol(t);
        insertNode(t->child[0]);
        break;
      case CallK:
        referenceSymbol(t);
        insertNode(t->child[0]);
        break;
      }
      break;
    case DeclK:
      switch(t->kind.decl)
      {
      case VarDeclK:
        registerSymbol(t, Variable, FALSE, t->child[0]->type);
        insertNode(t->child[0]);
        break;
      case ArrDeclK:
        registerSymbol(t, Variable, TRUE, t->child[0]->type);
        insertNode(t->child[0]);
        insertNode(t->child[1]);
        break;
      case FunDeclK:
        flag_functionDeclared = 1;
        registerSymbol(t, Function, FALSE, t->child[0]->type);
        incrementScope();
        insertNode(t->child[0]);
        insertNode(t->child[1]);
        insertNode(t->child[2]);
        decrementScope();
        flag_functionDeclared = 0;
        break;
      }
    case TypeK:
      break;
    case ParamK:
      switch (t->kind.param)
      {
      case VarParamK:
        registerSymbol(t, Parameter, FALSE, t->child[0]->type);
        insertNode(t->child[0]);
        break;
      case ArrParamK:
        registerSymbol(t, Parameter, TRUE, t->child[0]->type);
        insertNode(t->child[0]);
        break;
      case VoidParamK:
        break;
      }
      break;
    }
    t = t->sibling;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode *syntaxTree)
{
  initSymTab();
  insertNode(syntaxTree);
  if (TraceAnalyze)
  {
    printSymTab(listing);
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode *t)
{
  switch (t->nodekind)
  {
  case ExpK:
    switch (t->kind.exp)
    {
    case OpK:
      if ((t->child[0]->type != Integer) ||
          (t->child[1]->type != Integer))
        typeError(t, "Op applied to non-integer");
      t->type = Integer;
      break;
    case ConstK:
      t->type = Integer;
    case VarK:
    case ArrK:
//      t->type = t->child[0]->type;
      break;
    default:
      break;
    }
    break;
  case StmtK:
    switch (t->kind.stmt)
    {
    case SelectionK:
      if (t->child[0]->type != Integer)
        typeError(t->child[0], "if condition is not Integer");
      break;
    case IterationK:
      if (t->child[0]->type != Integer)
        typeError(t->child[0], "while condition is not Integer");
      break;
    case ReturnK:
      if (t->child[0]->type != Integer)
        typeError(t->child[0], "return value is not Integer");
      break;
    case AssignK:
      if (t->child[0]->type != t->child[1]->type)
        typeError(t->child[1], "assign type does not match");
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode *syntaxTree)
{
  traverse(syntaxTree, nullProc, checkNode);
}
