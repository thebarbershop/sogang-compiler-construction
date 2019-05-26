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

/* SIZE is the size of the hash table */
enum
{
  HASHTABLE_SIZE = 211
};

/* SHIFT is the power of two used as multiplier
   in hash function  */
enum
{
  HASH_SHIFT = 4
};

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
{
  int lineno;
  struct LineListRec *next;
} * LineList;

/* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
{
  char *name;
  LineList lines;
  int memloc; /* memory location for variable */
  struct BucketListRec *next;
} * BucketList;

/* Each scope has its own hash table.
 * The whole symbol table is managed as
 * a doubly linked list of scope-wide hash tables.
 */
typedef struct SymbolTableRec
{
  int depth; /* global scope is of depth 0,
              * and each compound statement increses depth by 1
              */
  BucketList hashTable[HASHTABLE_SIZE];
  struct SymbolTableRec *prev, *next;
  int location; /* memory location index */
} * SymbolTable;

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE *listing);

/* Initializes the global static variable globalSymbolTable */
void initSymTab();

/* increment current scope */
void incrementScope();

/* decrement scope */
void decrementScope();

/* Attempts to register symbol name. return 1 for success, 0 for failure. */
int registerSymbol(TreeNode *t);

/* Attempts to reference symbol name. return 1 for success, 0 for failure. */
int referenceSymbol(TreeNode *t);

#endif
