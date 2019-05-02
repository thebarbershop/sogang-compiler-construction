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
      for (i = 0; tokenString[i]; i++) {
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
  default: /* should never happen */
    fprintf(listing, "Unknown token: %d from \"%s\" at line %d\n", token, tokenString, lineno);
  }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode *newStmtNode(StmtKind kind)
{
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else
  {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode *newExpNode(ExpKind kind)
{
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else
  {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
  }
  return t;
}

/* Function newDeclNode creates a new declaration
 * node for syntax tree construction
 */
TreeNode *newDeclNode(DeclKind kind)
{
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else
  {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = DeclK;
    t->kind.decl = kind;
    t->lineno = lineno;
  }
  return t;
}

TreeNode *newTypeNode(TypeKind kind)
{
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else
  {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = TypeK;
    t->kind.type = kind;
    t->lineno = lineno;
  }
  return t;
}

TreeNode *newParamNode(ParamKind kind)
{
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else
  {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ParamK;
    t->kind.param = kind;
    t->lineno = lineno;
  }
  return t;
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
  for (i = 0; i < indentno; i++)
    fprintf(listing, " ");
}

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode *tree)
{
  int i;
  INDENT;
  while (tree != NULL)
  {
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
      default:
        fprintf(listing, "Unknown StmtNode kind\n");
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
      default:
        fprintf(listing, "Unknown ExpNode kind\n");
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
      default:
        fprintf(listing, "Unknown DeclNode kind\n");
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
      default:
        fprintf(listing, "Unknown TypeNode kind\n");
        break;
      }
    }
    else if (tree->nodekind == ParamK)
    {
      switch (tree->kind.param)
      {
      case VarParamK:
        fprintf(listing, "Parameter: variable %s\n", tree->attr.name);
        break;
      case ArrParamK:
        fprintf(listing, "Parameter: array %s\n", tree->attr.name);
        break;
      case VoidParamK:
        fprintf(listing, "Parameter: void\n");
        break;
      default:
        fprintf(listing, "Unknown ParameterNode kind\n");
        break;
      }
    }
    else
      fprintf(listing, "Unknown node kind\n");
    for (i = 0; i < MAXCHILDREN; i++)
      printTree(tree->child[i]);
    tree = tree->sibling;
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
    default:    return ""  ;
  }
}