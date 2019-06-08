/****************************************************/
/* File: analyze.h                                  */
/* Semantic analyzer interface for C- compiler      */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode *);

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode *);

/* Function mainCheck finds the main function
 * and asserts it is sematically sound.
 */
TreeNode* mainCheck(TreeNode *);

#endif
