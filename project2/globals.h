/****************************************************/
/* File: globals.h                                  */
/* Global types and vars for C- compiler            */
/* must come before other include files             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 6

/* Match the order of cmin.l */
typedef enum
/* book-keeping tokens */
{
   ENDFILE,
   ERROR,
   /* reserved words */
   ELSE,
   IF,
   INT,
   RETURN,
   VOID,
   WHILE,
   /* special symbols */
   PLUS,
   MINUS,
   TIMES,
   OVER,
   LT,
   LTE,
   GT,
   GTE,
   EQ,
   NEQ,
   ASSIGN,
   SEMI,
   COMMA,
   LPAREN,
   RPAREN,
   LBRACKET,
   RBRACKET,
   LBRACE,
   RBRACE,
   /* multicharacter tokens */
   ID,
   NUM
} TokenType;

extern FILE *source;  /* source code text file */
extern FILE *listing; /* listing output text file */
extern FILE *code;    /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

/**************************************************/
/* The following lines are copied from scan.h     */

#define MAXTOKENLEN 40

/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN + 1];

/* function getToken returns the next token in source file */
TokenType getToken(void);
/**************************************************/

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum
{
   StmtK,
   ExpK
} NodeKind;
typedef enum
{
   IfK,
   RepeatK,
   AssignK,
   ReadK,
   WriteK
} StmtKind;
typedef enum
{
   OpK,
   ConstK,
   IdK
} ExpKind;

/* ExpType is used for type checking */
typedef enum
{
   Void,
   Integer,
   Boolean
} ExpType;

#define MAXCHILDREN 3

typedef struct treeNode
{
   struct treeNode *child[MAXCHILDREN];
   struct treeNode *sibling;
   int lineno;
   NodeKind nodekind;
   union {
      StmtKind stmt;
      ExpKind exp;
   } kind;
   union {
      TokenType op;
      int val;
      char *name;
   } attr;
   ExpType type; /* for type checking of exps */
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error;
#endif
