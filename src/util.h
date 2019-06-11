/****************************************************/
/* File: util.h                                     */
/* Utility functions for the C- compiler            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

#include "globals.h"

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken(TokenType, const char *);

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char *copyString(char *);

char *getOp(TokenType);

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode *);

/* function getToken returns the next token in source file */
TokenType getToken(void);

/* Free all ast nodes and related pointers */
void destroyTree(TreeNode *);

/* Destroys all memory used by scanner.
 * Implemented in lex.yy.c */
int yylex_destroy(void);

/* Returns the position of last dot
 * If no dot in string, returns length of it */
int getBaseIndex(const char *fullPath);

#endif
