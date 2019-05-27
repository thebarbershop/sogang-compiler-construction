/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the C- compiler  */
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* the hash function which returns a number in [0, SIZE) */
static int hash(char *key)
{
  int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  {
    temp = ((temp << HASH_SHIFT) + key[i]) % HASHTABLE_SIZE;
    ++i;
  }
  return temp;
}

static void scopeError(TreeNode *t, const char *message)
{
  char * kindtype;
  switch (t->kind.exp) {
  case ArrK:
    kindtype = "Array";
    break;
  case VarK:
    kindtype = "Variable";
    break;
  case CallK:
    kindtype = "Function";
    break;
  default:
    break;
  }
  fprintf(listing, "%s %s at line %d: %s\n", kindtype, t->attr.name, t->lineno, message);
  Error=TRUE;
}

/* the symbol table */
static SymbolTable globalSymbolTable, currentScopeSymbolTable;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
static BucketList st_insert(char *name, int lineno, int loc)
{
  int h = hash(name);
  BucketList l = currentScopeSymbolTable->hashTable[h];
  while ((l != NULL) && (strcmp(name, l->name) != 0))
    l = l->next;
  if (l == NULL) /* symbol not yet in table */
  {
    l = malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->lines = malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = currentScopeSymbolTable->hashTable[h];
    currentScopeSymbolTable->hashTable[h] = l;
  }
  else /* found in table, so just add line number */
  {
    LineList t = l->lines;
    while (t->next != NULL)
    {
      if (t->lineno == lineno)
        return l; /* skip if lineno is already there */
      t = t->next;
    }
    t->next = malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
  return l;
} /* st_insert */

/* Attempts to reference symbol name. return 1 for success, 0 for failure. */
int referenceSymbol(TreeNode *t)
{
  SymbolTable original_currentScopeSymbolTable = currentScopeSymbolTable;
  while(currentScopeSymbolTable)
  {
    int h = hash(t->attr.name);
    BucketList l = currentScopeSymbolTable->hashTable[h];
    while ((l != NULL) && (strcmp(t->attr.name, l->name) != 0))
      l = l->next;

    if (l == NULL)
      currentScopeSymbolTable = currentScopeSymbolTable->prev;
    else
    {
      st_insert(t->attr.name, t->lineno, 0);
      currentScopeSymbolTable = original_currentScopeSymbolTable;
      return 1;
    }
  }
  scopeError(t, "used without declaration");
  return 0;
}

const char* const symbol_class_string[] = {"Variable", "Parameter", "Function"};
const char* const exp_type_string[] = {"void", "int"};

/* Attempts to register symbol name. return 1 for success, 0 for failure. */
int registerSymbol(TreeNode *t, SymbolClass symbol_class, int is_array, ExpType type) {
  int h = hash(t->attr.name);
  BucketList l = currentScopeSymbolTable->hashTable[h];
  while ((l != NULL) && (strcmp(t->attr.name, l->name) != 0))
    l = l->next;

  if (l == NULL)
  { /* The symbol is not found in current scope. */
    BucketList symbol = st_insert(t->attr.name, t->lineno, currentScopeSymbolTable->location++);
    symbol->symbol_class = symbol_class;
    symbol->is_array = is_array;
    symbol->type = type;
    if(is_array && symbol_class == Variable)
      symbol->array_size = t->child[1]->attr.val;
    if (t->nodekind == DeclK && t->kind.decl == ArrDeclK)
    {
      currentScopeSymbolTable->location += (t->child[1]->attr.val-1);
    }
    return 1;
  }
  else
  {
    char buffer[256];
    sprintf(buffer, "already declared.");
    scopeError(t, buffer);
    return 0;
  }
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE *listing)
{ 
  int i;
  fprintf(listing, "Symbol Name  Scope  Location  Symbol Class  Array?  Array Size  Expression Type  Line Numbers\n");
  fprintf(listing, "---------------------------------------------------------------------------------------------\n");
  for (i = 0; i < HASHTABLE_SIZE; ++i)
  {
    if (currentScopeSymbolTable->hashTable[i] != NULL)
    {
      BucketList l = currentScopeSymbolTable->hashTable[i];
      while (l != NULL)
      {
        LineList t = l->lines;
        fprintf(listing, "%-12s ", l->name);
        fprintf(listing, "%-6d ", currentScopeSymbolTable->depth);
        fprintf(listing, "%-9d ", l->memloc);
        fprintf(listing, "%-13s ", symbol_class_string[l->symbol_class]);
        fprintf(listing, "%-7s ", l->is_array?"Y":"N");
        if (l->is_array)
          fprintf(listing, "%-11d ", l->array_size);
        else
          fprintf(listing, "%-11s ", "-");
        fprintf(listing, "%-16s ", exp_type_string[l->type]);
        while (t != NULL)
        {
          fprintf(listing, "%4d ", t->lineno);
          t = t->next;
        }
        fprintf(listing, "\n");
        l = l->next;
      }
    }
  }
  fprintf(listing, "\n");
} /* printSymTab */

/* Initializes the global static variable globalSymbolTable */
void initSymTab()
{
  globalSymbolTable = malloc(sizeof(struct SymbolTableRec));
  globalSymbolTable->depth = 0;
  globalSymbolTable->prev = NULL;
  globalSymbolTable->next = NULL;
  for(int i = 0; i < HASHTABLE_SIZE; ++i)
    globalSymbolTable->hashTable[i] = NULL;
  currentScopeSymbolTable = globalSymbolTable;
}

/* increment current scope */
void incrementScope(int reset_location)
{
  currentScopeSymbolTable->next = malloc(sizeof(struct SymbolTableRec));
  currentScopeSymbolTable->next->depth = currentScopeSymbolTable->depth + 1;
  currentScopeSymbolTable->next->prev = currentScopeSymbolTable;
  currentScopeSymbolTable->next->next = NULL;
  currentScopeSymbolTable->next->location = reset_location?0:currentScopeSymbolTable->location;
  for(int i = 0; i < HASHTABLE_SIZE; ++i)
    currentScopeSymbolTable->next->hashTable[i] = NULL;
  currentScopeSymbolTable = currentScopeSymbolTable->next;

}

/* decrement scope */
void decrementScope()
{
  for(int i = 0; i < HASHTABLE_SIZE; ++i)
    if(currentScopeSymbolTable->hashTable[i])
      free(currentScopeSymbolTable->hashTable[i]);
  currentScopeSymbolTable = currentScopeSymbolTable->prev;
}