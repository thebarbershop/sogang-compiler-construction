/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the C- compiler       */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
enum
{
  HASH_SHIFT = 4
};

enum
{
  WORD_SIZE = 4
};

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE *listing);

/* Initializes the global static variable currentScopeSymbolTable
 * to represent the global scope */
void initSymTab(void);

/* increment current scope */
void incrementScope(void);

/* decrement scope */
void decrementScope(void);

/* Attempts to register symbol name. return 1 for success, 0 for failure. */
int registerSymbol(TreeNode *t, SymbolClass symbol_class, int is_array, ExpType type);

/* Attempts to lookup symbol from table. Return table entry or NULL. */
BucketList lookupSymbol(TreeNode *t);

/* set memory location of current symbol table */
void setCurrentScopeMemoryLocation(int);

/* Adds global symbols for pre-defined IO functions.
 * Leaves TreeNode pointer 'IOTreeNodes' to input function,
 * whose sibling is the output function. */
void addIO(void);

#endif
