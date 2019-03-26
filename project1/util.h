/****************************************************/
/* File: util.h                                     */
/* Utility functions for the C- compiler            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken(TokenType, const char *);

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode *newStmtNode(StmtKind);

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode *newExpNode(ExpKind);

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char *copyString(char *);

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode *);

/* The following lines are copied from scan.h */

/* function getToken returns the 
 * next token in source file
 */
#define MAXTOKENLEN 40

/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN + 1];
TokenType getToken(void);

#endif
