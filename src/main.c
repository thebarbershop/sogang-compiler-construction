/****************************************************/
/* File: main.c                                     */
/* Main program for C- compiler                     */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#include "globals.h"  
#include "util.h"
#include <assert.h>

/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */
#define NO_CODE TRUE

#if NO_PARSE
#else
#include "parse.h" 
#include "y.tab.h"
#if !NO_ANALYZE
#include "symtab.h"
#include "analyze.h"
#if !NO_CODE
//#include "cgen.h"
#endif
#endif
#endif

/* allocate global variables */
int lineno = 0;
FILE *source;
FILE *listing;
FILE *code;

/* allocate and set tracing flags */
int TraceScan = FALSE;
int TraceParse = FALSE;
int TraceAnalyze = TRUE;
int TraceCode = FALSE;

int Error = FALSE;

int main(int argc, char *argv[])
{
  int i;
  #if !NO_PARSE
  TreeNode *syntaxTree;
  #endif
  char pgm[120]; /* source code file name */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <filename>\n", argv[0]);
    exit(1);
  }
  strcpy(pgm, argv[1]);
  if (strchr(pgm, '.') == NULL)
    strcat(pgm, ".c");
  source = fopen(pgm, "r");
  if (source == NULL)
  {
    fprintf(stderr, "File %s not found\n", pgm);
    exit(1);
  }
  listing = stdout; /* send listing to screen */
  if(TraceScan) {
    fprintf(listing, "\tline number\ttoken\t\tlexeme\n");
    for(i = 0; i < 54; ++i)
      fputc('-', listing);
    fputc('\n', listing);
  }
#if NO_PARSE
  while (getToken() != ENDFILE)
    ;
#else
  syntaxTree = parse();
  if (TraceParse && !Error)
  {
    fprintf(listing, "Syntax tree:\n");
    printTree(syntaxTree);
  }
#if !NO_ANALYZE
  if (!Error)
  {
    if(TraceAnalyze)
      fprintf(listing, "Building Symbol Tree..\n\n");
    buildSymtab(syntaxTree);
    destroyGlobalSymbolTable();
  }
  if(!Error)
  {
      fprintf(listing, "Performing Type Check..\n");
      typeCheck(syntaxTree);
      fprintf(listing, "Finished Type Check\n");
  }
#if !NO_CODE
  if (!Error)
  {
    char *codefile;
    int fnlen = strcspn(pgm, ".");
    codefile = (char *)calloc(fnlen + 4, sizeof(char));
    strncpy(codefile, pgm, fnlen);
    strcat(codefile, ".tm");
    code = fopen(codefile, "w");
    if (code == NULL)
    {
      printf("Unable to open %s\n", codefile);
      exit(1);
    }
    codeGen(syntaxTree, codefile);
    fclose(code);
  }
#endif
#endif
#endif
  fclose(source);
  destroyTree(syntaxTree);
  yylex_destroy();

  return 0;
}
