/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the C- compiler                              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken(TokenType token, const char *tokenString)
{
  switch (token)
  {
  /* book-keeping tokens */
  case ENDFILE:
    fprintf(listing, "\t\tEOF\n");
    break;
  case ERROR:
    fprintf(listing,
            "\t\tERROR\t\t%s\n", tokenString);
    break;

  /* reserved words */
  case ELSE:
  case IF:
  case INT:
  case RETURN:
  case VOID:
  case WHILE:
    {
      int i;
      fprintf(listing, "\t\t");
      for (i = 0; tokenString[i]; ++i) {
        fputc(tokenString[i]-0x20, listing);
      }
      fprintf(listing, "\t\t%s\n", tokenString);
      break;
    }
  /* special symbols */
  case PLUS:
  case MINUS:
  case TIMES:
  case OVER:
  case LT:
  case LTE:
  case GT:
  case GTE:
  case EQ:
  case NEQ:
  case ASSIGN:
  case SEMI:
  case COMMA:
  case LPAREN:
  case RPAREN:
  case LBRACKET:
  case RBRACKET:
  case LBRACE:
  case RBRACE:
    fprintf(listing, "\t\t%s\t\t%s\n", tokenString, tokenString);
    break;

  /* multicharacter tokens */
  case NUM:
    fprintf(listing, "\t\tNUM\t\t%s\n", tokenString);
    break;
  case ID:
    fprintf(listing, "\t\tID\t\t%s\n", tokenString);
    break;
  }
}


/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char *copyString(char *s)
{
  int n;
  char *t;
  if (s == NULL)
    return NULL;
  n = (int)strlen(s) + 1;
  t = malloc((size_t)n);
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else
    strcpy(t, s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno += 2
#define UNINDENT indentno -= 2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{
  int i;
  for (i = 0; i < indentno; ++i)
    fprintf(listing, " ");
}

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode *tree)
{
  int i;
  INDENT;
  int sibling = 0;
  while (tree != NULL)
  {
    if(!sibling && tree->sibling != NULL) {
      printSpaces();
      fprintf(listing, "(\n");
      INDENT;
    }
    printSpaces();
    if (tree->nodekind == StmtK)
    {
      switch (tree->kind.stmt)
      {
      case CompoundK:
        fprintf(listing, "Compound Statement\n");
        break;
      case SelectionK:
        fprintf(listing, "Selection Statement\n");
        break;
      case IterationK:
        fprintf(listing, "Iteration Statement\n");
        break;
      case ReturnK:
        fprintf(listing, "Return Statement\n");
        break;
      }
    }
    else if (tree->nodekind == ExpK)
    {
      switch (tree->kind.exp)
      {
      case AssignK:
        fprintf(listing, "Assign Expression\n");
        break;
      case OpK:
        fprintf(listing, "Op: %s\n", getOp(tree->attr.op));
        break;
      case ConstK:
        fprintf(listing, "Const: %d\n", tree->attr.val);
        break;
      case VarK:
        fprintf(listing, "Variable: %s\n", tree->attr.name);
        break;
      case ArrK:
        fprintf(listing, "Array: %s\n", tree->attr.name);
        break;
      case CallK:
        fprintf(listing, "Calling: %s\n", tree->attr.name);
        break;
      }
    }
    else if (tree->nodekind == DeclK)
    {
      switch (tree->kind.decl)
      {
      case VarDeclK:
        fprintf(listing, "Variable Declaration: %s\n", tree->attr.name);
        break;
      case ArrDeclK:
        fprintf(listing, "Array Declaration: %s\n", tree->attr.name);
        break;
      case FunDeclK:
        fprintf(listing, "Function Declaration: %s\n", tree->attr.name);
        break;
      }
    }
    else if (tree->nodekind == TypeK)
    {
      switch (tree->kind.type)
      {
      char *type;
      case TypeGeneralK:
        if(tree->type == Integer) {
          type = "int";
        }
        else {
          type = "void";
        }
        fprintf(listing, "Type: %s\n", type);
        break;
      }
    }
    else if (tree->nodekind == ParamK)
    {
      switch (tree->kind.param)
      {
      case VarParamK:
        fprintf(listing, "Parameter (variable): %s\n", tree->attr.name);
        break;
      case ArrParamK:
        fprintf(listing, "Parameter (array): %s\n", tree->attr.name);
        break;
      case VoidParamK:
        fprintf(listing, "Parameter: void\n");
        break;
      }
    }
    else
      fprintf(listing, "Unknown node kind\n");
    for (i = 0; i < MAXCHILDREN; ++i)
      printTree(tree->child[i]);
    tree = tree->sibling;
    if(sibling && tree == NULL){
      UNINDENT;
      printSpaces();
      fprintf(listing, ")\n");
    }
    if(tree!=NULL) {
      ++sibling;
    }
  }
  UNINDENT;
}

/* Return string for the given speration */
char *getOp(TokenType op) {
  switch(op) {
    case PLUS:  return "+" ;  
    case MINUS: return "-" ;  
    case TIMES: return "*" ;  
    case OVER:  return "/" ;  
    case LT:    return "<" ;  
    case LTE:   return "<=";  
    case GT:    return ">" ;  
    case GTE:   return ">=";    
    case EQ:    return "==";
    case NEQ:   return "!=";
  }
  return "";
}

/* Free all ast nodes and related pointers */
void destroyTree(TreeNode * syntaxTree) {

}
