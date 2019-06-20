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
#include "parse.h"
#include "util.h"

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
  char *kindtype = "";
  if (t->nodekind == ExpK)
  {
    switch (t->kind.exp)
    {
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
  }
  else if (t->nodekind == DeclK)
  {
    switch (t->kind.decl)
    {
    case ArrDeclK:
      kindtype = "Array";
      break;
    case VarDeclK:
      kindtype = "Variable";
      break;
    case FunDeclK:
      kindtype = "Function";
      break;
    }
  }
  else if (t->nodekind == ParamK)
  {
    switch (t->kind.decl)
    {
    case ArrParamK:
      kindtype = "Array Parameter";
      break;
    case VarParamK:
      kindtype = "Variable Parameter";
      break;
    case VoidParamK:
      return;
    }
  }
  fprintf(listing, "Scope Error at line %d: %s %s %s\n", t->lineno, kindtype, t->attr.name, message);
  Error = TRUE;
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
  while ((l != NULL) && (strcmp(name, l->treeNode->attr.name) != 0))
    l = l->next;
  if (l == NULL) /* symbol not yet in table */
  {
    l = malloc(sizeof(struct BucketListRec));
    addPtr(l);
    l->lines = malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->lines->next = NULL;
    l->memloc = loc;
    l->size = 0;
    l->is_registered_argument = 0;
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
  while (currentScopeSymbolTable)
  {
    BucketList l = currentScopeSymbolTable->hashTable[h];
    while ((l != NULL) && (strcmp(t->attr.name, l->treeNode->attr.name) != 0))
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

const char *const symbol_class_string[] = {"Variable", "Variable", "Parameter", "Function"};
const char *const exp_type_string[] = {"void", "int"};

/* Attempts to register symbol name. return 0 for success, 1 for failure. */
int registerSymbol(TreeNode *t, SymbolClass symbol_class, int is_array, ExpType type)
{
  int memloc_coeff = (t->nodekind == ParamK) ? 1 : -1; /* positive offset for parameters; negative otherwise */

  int h = hash(t->attr.name);
  BucketList l = currentScopeSymbolTable->hashTable[h];
  while ((l != NULL) && (strcmp(t->attr.name, l->treeNode->attr.name) != 0))
    l = l->next;

  if (l == NULL)
  { /* The symbol is not found in current scope. */
    int location = currentScopeSymbolTable->location;
    BucketList symbol = st_insert(t->attr.name, t->lineno, location);
    if (!isGlobalScope())
    {
      if (is_array && symbol_class != Parameter)
        currentScopeSymbolTable->location += memloc_coeff * WORD_SIZE * t->child[1]->attr.val;
      else
        currentScopeSymbolTable->location += memloc_coeff * WORD_SIZE;
    }
    symbol->symbol_class = symbol_class;
    symbol->is_array = is_array;
    if (is_array && (symbol_class == Global || symbol_class == Local))
      symbol->size = t->child[1]->attr.val;
    if (t->nodekind == DeclK || t->nodekind == ParamK)
      symbol->treeNode = t;
    if (t->nodekind == DeclK && t->kind.decl == ArrDeclK)
      symbol->memloc -= (symbol->size - 1) * WORD_SIZE;
    t->symbol = symbol;
    t->type = type;
    return 0;
  }
  else
  {
    char buffer[256];
    sprintf(buffer, "already declared.");
    scopeError(t, buffer);
    return 1;
  }
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE *listing)
{
  int i;
  fprintf(listing, "Symbol Name  Scope  Offset  Stack  Class     Array  Param.  Type  Line Numbers\n");
  fprintf(listing, "------------------------------------------------------------------------------\n");
  for (i = 0; i < HASHTABLE_SIZE; ++i)
  {
    if (currentScopeSymbolTable->hashTable[i] != NULL)
    {
      BucketList l = currentScopeSymbolTable->hashTable[i];
      while (l != NULL)
      {
        LineList t;
        fprintf(listing, "%-12s ", l->treeNode->attr.name);        /* Symbol Name */
        fprintf(listing, "%5d  ", currentScopeSymbolTable->depth); /* Scope */
        if (isGlobalScope())
          fprintf(listing, "%6c  ", '-'); /* Offset */
        else
        {
          if (l->is_registered_argument)
            fprintf(listing, "$a%d     ", l->memloc); /* Offset */
          else
            fprintf(listing, "%6d  ", l->memloc); /* Offset */
        }
        if (l->symbol_class == Function)
          fprintf(listing, "%5d  ", l->memloc); /* Stack */
        else
          fprintf(listing, "%5c  ", '-');                                /* Stack */
        fprintf(listing, "%-9s ", symbol_class_string[l->symbol_class]); /* Class */
        if (l->is_array)
          fprintf(listing, "%5d  ", l->size); /* Array */
        else
          fprintf(listing, "%5c  ", '-'); /* Array */
        if (l->symbol_class == Function)
          fprintf(listing, "%6d  ", l->size); /* Param. */
        else
          fprintf(listing, "%6c  ", '-');                              /* Param. */
        fprintf(listing, "%-5s ", exp_type_string[l->treeNode->type]); /* Type */
        for (t = l->lines; t; t = t->next)
          fprintf(listing, "%4d ", t->lineno); /* LineNo */
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
  for (int i = 0; i < HASHTABLE_SIZE; ++i)
    currentScopeSymbolTable->hashTable[i] = NULL;
}

/* increment current scope */
void incrementScope(void)
{
  SymbolTable newSymbolTable = malloc(sizeof(struct SymbolTableRec));
  newSymbolTable->depth = currentScopeSymbolTable->depth + 1;
  newSymbolTable->prev = currentScopeSymbolTable;
  newSymbolTable->location = currentScopeSymbolTable->location;
  for (int i = 0; i < HASHTABLE_SIZE; ++i)
    newSymbolTable->hashTable[i] = NULL;

  currentScopeSymbolTable = newSymbolTable;
}

/* decrement scope */
void decrementScope(void)
{
  int i;
  /* Move symbol table pointer to previous scope */
  SymbolTable tableToDelete = currentScopeSymbolTable;
  for (i = 0; i < HASHTABLE_SIZE; ++i)
  {
    while (tableToDelete->hashTable[i])
    {
      while (tableToDelete->hashTable[i]->lines)
      {
        LineList lineToDestroy = tableToDelete->hashTable[i]->lines;
        tableToDelete->hashTable[i]->lines = tableToDelete->hashTable[i]->lines->next;
        free(lineToDestroy);
      }
      tableToDelete->hashTable[i] = tableToDelete->hashTable[i]->next;
    }
  }
  currentScopeSymbolTable = currentScopeSymbolTable->prev;
  free(tableToDelete);
}

/* Sets memory location of current symbol table */
void setCurrentScopeMemoryLocation(int location)
{
  currentScopeSymbolTable->location = location;
}

/* Gets memory location of current symbol table */
int getCurrentScopeMemoryLocation(void)
{
  return currentScopeSymbolTable->location;
}

static TreeNode *IOtreeNodes;
/* Adds global symbols for pre-defined IO functions.
 * Returns pointer to input function node,
 * whose sibling is the output function node. */
void addIO(void)
{
  TreeNode *inputNode, *outputNode;
  char *buffer;

  {
    /* Register int input(void); to global symbol table */
    buffer = malloc(sizeof(char) * 6);
    strncpy(buffer, "input", 6);
    addPtr(buffer);
    BucketList symbol = st_insert(buffer, -1, 0);
    symbol->symbol_class = Function;
    symbol->is_array = FALSE;
    symbol->size = 0;

    /* Create a dummy tree node for input */
    inputNode = newDeclNode(FunDeclK);
    TreeNode *typeNode = newTypeNode(TypeGeneralK);
    TreeNode *paramNode = newParamNode(VoidParamK);
    TreeNode *stmtNode = newStmtNode(CompoundK);
    inputNode->lineno = typeNode->lineno = paramNode->lineno = stmtNode->lineno = -1;
    inputNode->child[0] = typeNode;
    inputNode->child[1] = paramNode;
    inputNode->child[2] = stmtNode;
    inputNode->attr.name = "input";
    inputNode->type = Integer;
    typeNode->type = Integer;
    symbol->treeNode = inputNode;
    inputNode->symbol = symbol;
  }

  {
    /* Register void output(int num); to global symbol table */
    buffer = malloc(sizeof(char) * 7);
    strncpy(buffer, "output", 7);
    addPtr(buffer);
    BucketList symbol = st_insert(buffer, -1, 0);
    symbol->symbol_class = Function;
    symbol->is_array = FALSE;
    symbol->size = 1;

    /* Create a dummy tree node for output */
    outputNode = newDeclNode(FunDeclK);
    TreeNode *typeNode = newTypeNode(TypeGeneralK);
    TreeNode *paramNode = newParamNode(VarParamK);
    TreeNode *stmtNode = newStmtNode(CompoundK);
    TreeNode *paramTypeNode = newTypeNode(TypeGeneralK);
    BucketList paramSymbol = malloc(sizeof(struct BucketListRec));
    addPtr(paramSymbol);
    outputNode->lineno = typeNode->lineno = paramNode->lineno = stmtNode->lineno = -1;
    outputNode->child[0] = typeNode;
    outputNode->child[1] = paramNode;
    outputNode->child[2] = stmtNode;
    outputNode->attr.name = "output";
    outputNode->type = Void;
    typeNode->type = Void;
    paramNode->child[0] = paramTypeNode;
    paramNode->type = Integer;
    paramNode->attr.name = "num";
    paramNode->symbol = paramSymbol;
    paramTypeNode->type = Integer;
    paramSymbol->treeNode = paramNode;
    paramSymbol->size = 0;
    paramSymbol->is_array = FALSE;
    paramSymbol->lines = NULL;
    paramSymbol->memloc = 4;
    paramSymbol->symbol_class = Parameter;

    symbol->treeNode = outputNode;
    outputNode->symbol = symbol;
  }

  inputNode->sibling = outputNode;
  IOtreeNodes = inputNode;
}

/* Returns TRUE if current scope is global
 * and FALSE otherwise. */
int isGlobalScope(void)
{
  return (currentScopeSymbolTable->depth == 0) ? 1 : 0;
}
