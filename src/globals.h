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

#ifndef YYPARSER
#include "y.tab.h"
#define ENDFILE 0
#define ERROR 65535
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 6

typedef int TokenType;

extern FILE *source;  /* source code text file */
extern FILE *listing; /* listing output text file */
extern FILE *code;    /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

/**************************************************/
/* The following lines are copied from scan.h     */
#define MAXTOKENLEN 40

/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN + 1];
/**************************************************/

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum
{
   StmtK,
   ExpK,
   DeclK,
   TypeK,
   ParamK
} NodeKind;
typedef enum
{
   CompoundK,
   SelectionK,
   IterationK,
   ReturnK
} StmtKind;
typedef enum
{
   AssignK,
   OpK,
   ConstK,
   VarK,
   ArrK,
   CallK,
} ExpKind;
typedef enum
{
   VarDeclK,
   ArrDeclK,
   FunDeclK
} DeclKind;
typedef enum
{
   TypeGeneralK
} TypeKind;
typedef enum
{
   VarParamK,
   ArrParamK,
   VoidParamK
} ParamKind;

/* ExpType is used for type checking */
typedef enum
{
   Void,
   Integer
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
      DeclKind decl;
      TypeKind type;
      ParamKind param;
   } kind;
   union {
      TokenType op;        /* for operator */
      int val;             /* for constant */
      char *name;          /* for variable */
   } attr;
   ExpType type; /* for type checking of exps and decls */
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

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
