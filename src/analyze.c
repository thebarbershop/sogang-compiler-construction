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

static void argumentError(TreeNode *t, const char *function_name, const char *message)
{
  fprintf(listing, "Argument error for function %s at line %d: %s\n", function_name, t->lineno, message);
  Error = TRUE;
}

static void semanticError(TreeNode *t, const char *message)
{
  fprintf(listing, "Semantic error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
}

static int flag_functionDeclared = FALSE;
static int flag_callArguments = FALSE;
static TreeNode* node_currentFunction;
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
          int scope_incremented = FALSE;
          int function_scope = TRUE;
          if(!flag_functionDeclared)
          {
            incrementScope(t);
            scope_incremented = TRUE;
            function_scope = FALSE;
          }
          flag_functionDeclared = FALSE;
          insertNode(t->child[0]);
          insertNode(t->child[1]);
          if(TraceAnalyze)
          {
            if(function_scope)
              fprintf(listing, "\n** Symbol table for scope of function %s declared at at line %d\n", node_currentFunction->attr.name, node_currentFunction->lineno);
            else
              fprintf(listing, "\n** Symbol table for nested scope in function %s closed at line %d\n", node_currentFunction->attr.name, t->lineno);
            printSymTab(listing);
          }
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
        BucketList l = lookupSymbol(t);
        if(l != NULL) {
          if (l->symbol_class == Function)
              typeError(t, "used a function like a variable");
          else if (!flag_callArguments) {
            if (l->is_array)
              typeError(t, "used an array like a variable");
          }
          t->symbol = l;
        }
        break;
      }
      case ArrK:
      {
        BucketList l = lookupSymbol(t);
        if(l != NULL) {
          t->symbol = l;
          if(!l->is_array)
          typeError(t, "used a non-array like a array");
        }
        insertNode(t->child[0]);
        break;
      }
      case CallK:
      {
        BucketList l = lookupSymbol(t);
        if(l != NULL) {
          t->symbol = l;
          if(l->symbol_class != Function)
            typeError(t, "used a non-function like a function");
        }
        ++flag_callArguments;
        insertNode(t->child[0]); /* This takes care of arguments */
        --flag_callArguments;
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
        node_currentFunction = t;
        registerSymbol(t, Function, FALSE, t->child[0]->type);
        incrementScope(t);
        insertNode(t->child[0]);
        setCurrentScopeMemoryLocation(4);   /* memory offset for paramters starts before control link */
        insertNode(t->child[1]);            /* This child takes care of parameter declarations */
        setCurrentScopeMemoryLocation(-4);  /* memory offset for local symbols start after return address */
        flag_functionDeclared = TRUE;
        insertNode(t->child[2]);            /* This child takes care of function body */
        decrementScope();
        node_currentFunction = NULL;
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
    fprintf(listing, "\n** Symbol table for global scope\n");
    printSymTab(listing);
  }
}

/* Check number and types of
 * function parameters and call arguments
 */
static void checkArguments(TreeNode *function, TreeNode *call)
{
  /* Function parameters are in function->child[1] */
  /* Call arguments are in call->child[0]          */
  int counter_params = 0;
  int counter_args = 0;
  TreeNode *params = function->child[1];
  TreeNode *args = call->child[0];
  char buff[256];

  /* If VoidParamK, args MUST be NULL */
  if(params && params->kind.param == VoidParamK)
  {
    if(args)
      argumentError(args, function->attr.name, "This function does not take arguments.");
    return;
  }

  if(params) ++counter_params;
  if(args) ++counter_args;
  while(params && args)
  {
    /* VoidParamK: NEVER OK */
    /* VarParamK: AssignK, OpK, ConstK, ArrK이면 OK
                  VarK이면 lookup해서 !is_array이어야 OK
                  CallK이면 lookup해서 type==Integer이어야 OK*/
    /* ArrParamK: VarK이면서 lookup해서 is_array이어야 OK */
    switch(params->kind.param)
    {
      BucketList symbol;
      case VoidParamK:
        argumentError(args, function->attr.name, "This function does not take arguments.");
        return;
      case VarParamK:
          if(args->kind.exp == VarK)
          {
            symbol = params->symbol;
            if(symbol->is_array)
            {
              sprintf(buff, "Expected integer for argument %d, but received array.", counter_args);
              argumentError(args, function->attr.name, buff);
              return;
            }
          }
          else if(args->kind.exp == CallK)
          {
            symbol = params->symbol;
            if(symbol->type != Integer)
            {
              sprintf(buff, "Expected integer for argument %d, but received void function call.", counter_args);
              argumentError(args, function->attr.name, buff);
              return;
            }
          }
        break;
        case ArrParamK:
          if(args->kind.exp != VarK)
          {
              sprintf(buff, "Expected array for argument %d, but received something else.", counter_args);
              argumentError(args, function->attr.name, buff);
              return;
          }
          else
          {
            symbol = params->symbol;
            if(!symbol->is_array)
            {
              sprintf(buff, "Expected array for argument %d, but received variable.", counter_args);
              argumentError(args, function->attr.name, buff);
              return;
            }
          }
        break;
    }

    /* Move to next parameter & argument */
    params = params->sibling;
    args = args->sibling;
    if(params) ++counter_params;
    if(args) ++counter_args;
  }
  if(!params && args) /* Too many arguments */
  {
    while(1)
    {
      args = args->sibling;
      if(args) ++counter_args;
      else break;
    }
    sprintf(buff, "Too many arguments. %d expected, %d given.", counter_params, counter_args);
    semanticError(call, buff);
    return;
  }
  if(params && !args) /* Too few arguments */
  {
    while(1)
    {
      params = params->sibling;
      if(params) ++counter_params;
      else break;
    }
    sprintf(buff, "Too few arguments. %d expected, %d given.", counter_params, counter_args);
    semanticError(call, buff);
    return;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode *t)
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
        typeCheck(t->child[0]);
        typeCheck(t->child[1]);
        break;
      }
      case SelectionK:
        typeCheck(t->child[0]);
        typeCheck(t->child[1]);
        typeCheck(t->child[2]);
        if (t->child[0]->type != Integer)
          typeError(t->child[0], "If-condition is not int");
        break;
      case IterationK:
        typeCheck(t->child[0]);
        typeCheck(t->child[1]);
        if (t->child[0]->type != Integer)
          typeError(t->child[0], "While-condition is not int");
        break;
      case ReturnK:
        typeCheck(t->child[0]);
        if (t->child[0]->type != node_currentFunction->type)
          typeError(t->child[0], "Return value does not match function type");
        break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      {
      case AssignK:
        typeCheck(t->child[0]);
        typeCheck(t->child[1]);
        if (t->child[0]->type != t->child[1]->type)
          typeError(t->child[1], "Assign type does not match");
        t->type = t->child[0]->type;
        break;
      case OpK:
        typeCheck(t->child[0]);
        typeCheck(t->child[1]);
        if ((t->child[0]->type != Integer) || (t->child[1]->type != Integer))
          typeError(t, "Op applied to non-integer");
        t->type = Integer;
        break;
      case ConstK:
        t->type = Integer;
        break;
      case VarK:
        t->type = t->symbol->type;
        break;
      case ArrK:
        typeCheck(t->child[0]);
        if ((t->child[0]->type != Integer))
          typeError(t, "Array index in not integer");
        t->type = t->symbol->type;
        break;
      case CallK:
      {
        typeCheck(t->child[0]);;
        t->type = t->symbol->type;
        
        /* Check number and type of arguments for function call */
        checkArguments(t->symbol->treeNode, t);
        break;
      }
      }
      break;
    case DeclK:
      switch(t->kind.decl)
      {
      case VarDeclK:
        typeCheck(t->child[0]);
        if (t->child[0]->type == Void)
          typeError(t, "Invalid variable declaration of type void");
        break;
      case ArrDeclK:
        typeCheck(t->child[0]);
        if (t->child[0]->type == Void)
          typeError(t, "Invalid array declaration of type void");
        typeCheck(t->child[1]);
        break;
      case FunDeclK:
        node_currentFunction = t;
        typeCheck(t->child[0]);
        t->type = t->child[0]->type;
        typeCheck(t->child[1]);
        typeCheck(t->child[2]);
        node_currentFunction = NULL;

        break;
      }
    case TypeK:
      break;
    case ParamK:
      switch (t->kind.param)
      {
      case VarParamK:
        typeCheck(t->child[0]);
        if (t->child[0]->type == Void)
          typeError(t, "Invalid parameter of type void");
        break;
      case ArrParamK:
        typeCheck(t->child[0]);
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

/* Function mainCheck finds the main function
 * and asserts it is sematically sound.
 */
TreeNode* mainCheck(TreeNode * node) {
  while(node) {
    if(!strcmp(node->attr.name, "main")) {
      if(node->type != Void)
        semanticError(node, "Return type of function \'main\' must be void.");
      else if(node->child[1]->kind.param != VoidParamK)
        semanticError(node, "Parameter of function \'main\' must be void.");
      else if(node->sibling)
        semanticError(node, "Illegal global definition after function \'main\'.");
      return node;
    }
    node = node->sibling;
  }
  semanticError(node, "Reached EOF before find function \'main\'.");
  return NULL;
}