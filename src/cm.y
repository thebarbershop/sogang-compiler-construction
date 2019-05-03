/****************************************************/
/* File: cmin.y                                     */
/* The C- Yacc/Bison specification file             */
/* Based on tiny.y source code from                 */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *

static char * savedName; /* for use in assignments */
static TreeNode * savedTree; /* stores syntax tree for later return */

static int yylex(void);
int yyerror(char * message);

%}

%token ERROR
/* reserved words */
%token ELSE IF INT RETURN VOID WHILE
/* special symbols */
%token PLUS MINUS TIMES OVER LT LTE GT GTE EQ NEQ ASSIGN SEMI COMMA LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
/* multicharacter tokens */
%token ID NUM

%% /* Grammar for C- */

program             : declaration_list
                        { savedTree = $1;}
                    ;
identifier          : ID
                        { savedName = copyString(tokenString); }
                    ;
number              : NUM
                        {
                            $$ = newExpNode(ConstK);
                            $$->attr.val = atoi(tokenString);
                        }
                    ;
declaration_list    : declaration_list declaration
                        {
                            YYSTYPE t = $1;
                            if (t != NULL)
                            {
                                while (t->sibling != NULL)
                                {
                                    t = t->sibling;
                                }
                                t->sibling = $2;
                                $$ = $1;
                            }
                            else $$ = $2;
                        }
                    | declaration 
                        { $$ = $1; }
                    ;
declaration         : var_declaration
                        { $$ = $1; }
                    | fun_declaration
                        { $$ = $1; }
                    ;
var_declaration     : type_specifier identifier SEMI
                        {
                            $$ = newDeclNode(VarDeclK);
                            $$->child[0] = $1;
                            $$->attr.name = savedName;
                        }
                    | type_specifier identifier LBRACKET
                        {
                            $$ = newDeclNode(ArrDeclK);
                            $$->child[0] = $1;
                            $$->attr.name = savedName;
                        }
                        number RBRACKET SEMI
                        {
                            $$ = $4;
                            $$->child[1] = $5;
                        }
                    ;
type_specifier      : INT
                        {
                            $$ = newTypeNode(TypeGeneralK);
                            $$->type = Integer;
                        }
                    | VOID
                        {
                            $$ = newTypeNode(TypeGeneralK);
                            $$->type = Void;
                        }
                    ;
fun_declaration     : type_specifier identifier LPAREN
                        {
                            $$ = newDeclNode(FunDeclK);
                            $$->child[0] = $1;
                            $$->attr.name = savedName;
                        }
                        params RPAREN compound_stmt
                        {
                            $$ = $4;
                            $$->child[1] = $5; /* params */
                            $$->child[2] = $7; /* statements */
                        }
                    ;
params              : param_list
                        { $$ = $1; }
                    | VOID
                        {
                            $$ = newParamNode(VoidParamK);
                        }
                    ;
param_list          : param_list COMMA param
                        {
                            YYSTYPE t = $1;
                            if (t != NULL)
                            {
                                while (t->sibling != NULL)
                                {
                                    t = t->sibling;
                                }
                                t->sibling = $3;
                                $$ = $1;
                            }
                            else $$ = $3;
                        }
                    | param  { $$ = $1; }
                    ;
param               : type_specifier identifier
                        {
                            $$ = newParamNode(VarParamK);
                            $$->child[0] = $1;
                            $$->attr.name = savedName;
                        }
                    | type_specifier identifier LBRACKET RBRACKET
                        {
                            $$ = newParamNode(ArrParamK);
                            $$->child[0] = $1;
                            $$->attr.name = savedName;
                        }
                    ;
compound_stmt       : LBRACE local_declarations statement_list RBRACE
                        {
                            $$ = newStmtNode(CompoundK);
                            $$->child[0] = $2;
                            $$->child[1] = $3;
                        }
                    ;
local_declarations  : local_declarations var_declaration
                        {
                            YYSTYPE t = $1;
                            if (t != NULL)
                            {
                                while (t->sibling != NULL)
                                {
                                    t = t->sibling;
                                }
                                t->sibling = $2;
                                $$ = $1;
                            }
                            else $$ = $2;
                        }
                    | empty { $$ = $1; }
                    ;
statement_list      : statement_list statement
                        {
                            YYSTYPE t = $1;
                            if (t != NULL)
                            {
                                while (t->sibling != NULL)
                                {
                                    t = t->sibling;
                                }
                                t->sibling = $2;
                                $$ = $1;
                            }
                            else $$ = $2;
                        }
                    | empty { $$ = $1; }
                    ;
statement           : expression_stmt { $$ = $1; }
                    | compound_stmt { $$ = $1; }
                    | selection_stmt { $$ = $1; }
                    | iteration_stmt {$$ = $1; }
                    | return_stmt { $$ = $1; }
                    ;
expression_stmt     : expression SEMI { $$ = $1; }
                    | SEMI { $$ = NULL; }
selection_stmt      : IF LPAREN expression RPAREN statement
                        {
                            $$ = newStmtNode(SelectionK);
                            $$->child[0] = $3;
                            $$->child[1] = $5;
                        }
                    | IF LPAREN expression RPAREN statement ELSE statement
                        {
                            $$ = newStmtNode(SelectionK);
                            $$->child[0] = $3;
                            $$->child[1] = $5;
                            $$->child[2] = $7;
                        }
                    ;
iteration_stmt      : WHILE LPAREN expression RPAREN statement
                        {
                            $$ = newStmtNode(IterationK);
                            $$->child[0] = $3;
                            $$->child[1] = $5;
                        }
                    ;
return_stmt         : RETURN SEMI
                        {
                            $$ = newStmtNode(ReturnK);
                        }
                    | RETURN expression SEMI
                        {
                            $$ = newStmtNode(ReturnK);
                            $$->child[0] = $2;
                        }
                    ;
expression          : var ASSIGN expression
                        {
                            $$ = newExpNode(AssignK);
                            $$->child[0] = $1;
                            $$->child[1] = $3;
                        }
                    | simple_expression { $$ = $1; }
                    ;
var                 : identifier
                        {
                            $$ = newExpNode(VarK);
                            $$->attr.name = savedName;
                        }
                    | identifier
                        {
                            $$ = newExpNode(ArrK);
                            $$->attr.name = savedName;
                        }
                    LBRACKET expression RBRACKET
                        {
                            $$ = $2;
                            $$->child[0] = $4;
                        }
                    ;
simple_expression   : additive_expression relop additive_expression
                        {
                            $$ = $2;
                            $$->child[0] = $1;
                            $$->child[1] = $3;
                        }
                    | additive_expression { $$ = $1; }
                    ;
relop               : LTE
                        {
                            $$ = newExpNode(OpK);
                            $$->attr.op = LTE;
                        }
                    | LT
                        {
                            $$ = newExpNode(OpK);
                            $$->attr.op = LT;
                        }
                    | GT
                        {
                            $$ = newExpNode(OpK);
                            $$->attr.op = GT;
                        }
                    | GTE
                        {
                            $$ = newExpNode(OpK);
                            $$->attr.op = GTE;
                        }
                    | EQ
                        {
                            $$ = newExpNode(OpK);
                            $$->attr.op = EQ;
                        }
                    | NEQ
                        {
                            $$ = newExpNode(OpK);
                            $$->attr.op = NEQ;
                        }
                    ;
additive_expression : additive_expression addop term
                        {
                            $$ = $2;
                            $$->child[0] = $1;
                            $$->child[1] = $3;
                        }
                    | term { $$ = $1; }
                    ;
addop               : PLUS
                        {
                            $$ = newExpNode(OpK);
                            $$->attr.op = PLUS;
                        }
                    | MINUS
                        {
                            $$ = newExpNode(OpK);
                            $$->attr.op = MINUS;
                        }
                    ;
term                : term mulop factor 
                        {
                            $$ = $2;
                            $$->child[0] = $1;
                            $$->child[1] = $3;
                        }
                    | factor { $$ =  $1; }
                    ;
mulop               : TIMES
                        {
                            $$ = newExpNode(OpK);
                            $$->attr.op = TIMES;
                        }
                    | OVER
                        {
                            $$ = newExpNode(OpK);
                            $$->attr.op = OVER;
                        }
                    ;
factor              : LPAREN expression RPAREN { $$ = $2; }
                    | var { $$ = $1; }
                    | call { $$ = $1; }
                    | number { $$ = $1; }
                    ;
call                : identifier
                        {
                            $$ = newExpNode(CallK);
                            $$->attr.name = savedName;
                        }
                    LPAREN args RPAREN
                        {
                            $$ = $2;
                            $$->child[0] = $4;
                        }
                    ;
args                : arg_list { $$ = $1; }
                    | empty { $$ = $1; }
                    ;
arg_list            : arg_list COMMA expression
                        {
                            YYSTYPE t = $1;
                            if (t != NULL)
                            {
                                while (t->sibling != NULL)
                                {
                                    t = t->sibling;
                                }
                                t->sibling = $3;
                                $$ = $1;
                            }
                            else $$ = $3;
                        }
                    | expression { $$ = $1; }
                    ;
empty               : /* epsilon */ { $$ = NULL; }
                    ;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ 
  return getToken();
}

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}
