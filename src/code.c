/****************************************************/
/* File: code.c                                     */
/* Code emitting utilities for the C- compiler      */
/* and interface to the SPIM machine                */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#include "globals.h"
#include "code.h"

/* Procedure emitBlank prints a blank line
 * in the code file.
 */
void emitBlank(void)
{
  if (TraceCode) fprintf(code, "\n");
}

/* Procedure emitComment prints a comment line 
 * with comment c in the code file
 */
void emitComment( char * c )
{ if (TraceCode && c && c[0]) fprintf(code,"# %s",c);
  fprintf(code, "\n");
}

/* Procedure emitCode prints a code line */
void emitCode(char *codeLine, char *comment)
{
 fprintf(code, "%s", codeLine);
 emitComment(comment);
}

/* Procedure emitRegImm prints a code line
 * that takes one register and one immidiate
 */
void emitRegImm(char *op, char *reg, int imm, char* comment)
{
  fprintf(code, "%s %s %d", op, reg, imm);
  emitComment(comment);
}

/* Procedure emitRegAddr prints a code line
 * that takes one register and one address
 */
void emitRegAddr(char *op, char *reg1, int offset, char *reg2, char *comment)
{
  fprintf(code, "%s %s ", op, reg1);
  if(offset)
    fprintf(code, "%d", offset);
  fprintf(code, "(%s)", reg2);
  emitComment(comment);
}

/* Procedure emitRegAddr prints a code line
 * that takes two register and one immidiate
 */
void emitRegRegImm(char *op, char *reg1, char *reg2, int imm, char *comment)
{
  fprintf(code, "%s %s %s %d", op, reg1, reg2, imm);
  emitComment(comment);
}

/* Procedure emitRegRegReg prints a code line
 * that takes one register
 */
void emitReg(char *op, char *reg, char *comment)
{
  fprintf(code, "%s %s", op, reg);
  emitComment(comment);
}

/* Procedure emitRegRegReg prints a code line
 * that takes two registers
 */
void emitRegReg(char *op, char *reg1, char *reg2, char *comment)
{
  fprintf(code, "%s %s %s", op, reg1, reg2);
  emitComment(comment);
}

/* Procedure emitRegRegReg prints a code line
 * that takes three registers
 */
void emitRegRegReg(char *op, char *reg1, char *reg2, char *reg3, char *comment)
{
  fprintf(code, "%s %s %s %s", op, reg1, reg2, reg3);
  emitComment(comment);
}