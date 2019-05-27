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

static int flag_functionDeclared = FALSE;
static int flag_callArguments = FALSE;
/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode(TreeNode *t)
{
  while(t)
  {
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
            incrementScope(t);
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
      {
        BucketList l = lookupSymbol(t, TRUE);
        if (l->symbol_class == Function)
            typeError(t, "used a function like a variable");
        else if (!flag_callArguments) {
          if (l->is_array)
            typeError(t, "used an array like a variable");
        }
        break;
      }
      case ArrK:
      {
        BucketList l = lookupSymbol(t, TRUE);
        if(!l->is_array)
          typeError(t, "used a non-array like a array");
        insertNode(t->child[0]);
        break;
      }
      case CallK:
      {
        BucketList l = lookupSymbol(t, TRUE);
        if(l->symbol_class != Function)
          typeError(t, "used a non-function like a function");
        flag_callArguments = TRUE;
        insertNode(t->child[0]); /* This takes care of arguments */
        flag_callArguments = FALSE;
        break;
      }
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
        incrementScope(t);
        insertNode(t->child[0]);
        t->scopeSymbolTable->location=4;  /* memory offset for paramters starts before control link */
        insertNode(t->child[1]);          /* This child takes care of parameter declarations */
        t->scopeSymbolTable->location=-4; /* memory offset for local symbols start after return address */
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
    printSymTab(listing);
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode *t)
{
  while(t)
  {
    switch (t->nodekind)
    {
    case StmtK:
      switch (t->kind.stmt)
      {
      case CompoundK:
      {
        int scope_incremented = 0;
        if(!flag_functionDeclared)
        {
          incrementScope(t);
          scope_incremented = 1;
        }
        else
          flag_functionDeclared = 0;

        checkNode(t->child[0]);
        checkNode(t->child[1]);
        if(scope_incremented)
          decrementScope();
        break;
      }
      case SelectionK:
        checkNode(t->child[0]);
        checkNode(t->child[1]);
        checkNode(t->child[2]);
        if (t->child[0]->type != Integer)
          typeError(t->child[0], "If-condition is not int");
        break;
      case IterationK:
        checkNode(t->child[0]);
        checkNode(t->child[1]);
        if (t->child[0]->type != Integer)
          typeError(t->child[0], "While-condition is not int");
        break;
      case ReturnK:
        checkNode(t->child[0]);
        if (t->child[0]->type != Integer)
          typeError(t->child[0], "Return value is not int");
        break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      {
      case AssignK:
        checkNode(t->child[0]);
        checkNode(t->child[1]);
        if (t->child[0]->type != t->child[1]->type)
          typeError(t->child[1], "Assign type does not match");
        break;
      case OpK:
        checkNode(t->child[0]);
        checkNode(t->child[1]);
        if ((t->child[0]->type != Integer) || (t->child[1]->type != Integer))
          typeError(t, "Op applied to non-integer");
        t->type = Integer;
        break;
      case ConstK:
        t->type = Integer;
        break;
      case VarK:
        t->type = lookupSymbol(t, FALSE)->type;
        break;
      case ArrK:
        checkNode(t->child[0]);
        t->type = lookupSymbol(t, FALSE)->type;
        if ((t->child[0]->type != Integer))
          typeError(t, "Array index in not integer");
        break;
      case CallK:
        checkNode(t->child[0]);
        break;
      }
      break;
    case DeclK:
      switch(t->kind.decl)
      {
      case VarDeclK:
        checkNode(t->child[0]);
        if (t->child[0]->type == Void)
          typeError(t, "Invalid variable declaration of type void");
        break;
      case ArrDeclK:
        checkNode(t->child[0]);
        if (t->child[0]->type == Void)
          typeError(t, "Invalid array declaration of type void");
        checkNode(t->child[1]);
        break;
      case FunDeclK:
        flag_functionDeclared = 1;
        incrementScope(t);
        checkNode(t->child[0]);
        checkNode(t->child[1]);
        checkNode(t->child[2]);
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
        checkNode(t->child[0]);
        if (t->child[0]->type == Void)
          typeError(t, "Invalid parameter of type void");
        break;
      case ArrParamK:
        checkNode(t->child[0]);
        if (t->child[0]->type == Void)
          typeError(t, "Invalid array parameter of type void");
        break;
      case VoidParamK:
        break;
      }
      break;
    }
    t = t->sibling;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode *syntaxTree)
{
  checkNode(syntaxTree);
}
