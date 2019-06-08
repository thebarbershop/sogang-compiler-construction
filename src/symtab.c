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
static int hash(const char *key)
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
  case AssignK:
  case OpK:
  case ConstK:
    break;
  }
  fprintf(listing, "Scope Error at line %d: %s %s %s\n", t->lineno, kindtype, t->attr.name, message);
  Error=TRUE;
}

/* the symbol table */
static SymbolTable currentScopeSymbolTable;

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
    l->lines->next = NULL;
    l->memloc = loc;
    l->array_size = 0;
    l->next = NULL;
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

/* Look up symbol name. return table entry or NULL. */
BucketList lookupSymbol(TreeNode *t)
{
  SymbolTable original_currentScopeSymbolTable = currentScopeSymbolTable;
  int h = hash(t->attr.name);
  while(currentScopeSymbolTable)
  {
    BucketList l = currentScopeSymbolTable->hashTable[h];
    while ((l != NULL) && (strcmp(t->attr.name, l->name) != 0))
      l = l->next;

    if (l == NULL)
      currentScopeSymbolTable = currentScopeSymbolTable->prev;
    else
    {
      st_insert(t->attr.name, t->lineno, 0);
      currentScopeSymbolTable = original_currentScopeSymbolTable;
      return l;
    }
  }
  scopeError(t, "used without declaration");
  currentScopeSymbolTable = original_currentScopeSymbolTable;
  return NULL;
}

const char* const symbol_class_string[] = {"Variable", "Parameter", "Function"};
const char* const exp_type_string[] = {"void", "int"};

/* Attempts to register symbol name. return 1 for success, 0 for failure. */
int registerSymbol(TreeNode *t, SymbolClass symbol_class, int is_array, ExpType type) {
  int memloc_coeff = (t->nodekind == ParamK)?1:-1; /* positive offset for parameters; negative otherwise */
  
  int h = hash(t->attr.name);
  BucketList l = currentScopeSymbolTable->hashTable[h];
  while ((l != NULL) && (strcmp(t->attr.name, l->name) != 0))
    l = l->next;

  if (l == NULL)
  { /* The symbol is not found in current scope. */
    BucketList symbol = st_insert(t->attr.name, t->lineno, currentScopeSymbolTable->location);
    currentScopeSymbolTable->location += memloc_coeff*WORD_SIZE;
    symbol->symbol_class = symbol_class;
    symbol->is_array = is_array;
    symbol->type = type;
    if(is_array && symbol_class == Variable)
      symbol->array_size = t->child[1]->attr.val;
    if (t->nodekind == DeclK && t->kind.decl == ArrDeclK)
      currentScopeSymbolTable->location += memloc_coeff*WORD_SIZE*(t->child[1]->attr.val-1);
    if (t->nodekind == DeclK && t->kind.decl == FunDeclK)
      symbol->treeNode = t;
    t->symbol = symbol;
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
  fprintf(listing, "Symbol Name  Scope  Location  Symbol Class  Array?  Array Size  Type  Line Numbers\n");
  fprintf(listing, "----------------------------------------------------------------------------------\n");
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
        fprintf(listing, "%-5s ", exp_type_string[l->type]);
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

/* Initializes the global static variable currentScopeSymbolTable
 * to represent the global scope */
void initSymTab(void)
{
  currentScopeSymbolTable = malloc(sizeof(struct SymbolTableRec));
  currentScopeSymbolTable->depth = 0;
  currentScopeSymbolTable->location = 0;
  currentScopeSymbolTable->prev = NULL;
  for(int i = 0; i < HASHTABLE_SIZE; ++i)
    currentScopeSymbolTable->hashTable[i] = NULL;
}

/* increment current scope */
void incrementScope(TreeNode *t)
{
  SymbolTable newSymbolTable = malloc(sizeof(struct SymbolTableRec));
  newSymbolTable->depth = currentScopeSymbolTable->depth + 1;
  newSymbolTable->prev = currentScopeSymbolTable;
  newSymbolTable->location = currentScopeSymbolTable->location;
  for(int i = 0; i < HASHTABLE_SIZE; ++i)
    newSymbolTable->hashTable[i] = NULL;

  currentScopeSymbolTable = newSymbolTable;
}

/* decrement scope */
void decrementScope(void)
{
  /* Move symbol table pointer to previous scope */
  SymbolTable tableToDelete = currentScopeSymbolTable;
  currentScopeSymbolTable = currentScopeSymbolTable->prev;
  free(tableToDelete);
}

/* set memory location of current symbol table */
void setCurrentScopeMemoryLocation(int location) {
  currentScopeSymbolTable->location = location;
}

/* destroys the global symbol table */
void destroyGlobalSymbolTable(void) {
  free(currentScopeSymbolTable);
}