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

/* Procedure emitComment prints a comment line 
 * with comment c in the code file
 */
void emitComment( char * c )
{ if (TraceCode && c && c[0]) fprintf(code,"# %s\n",c);
}

/* Procedure emitCode prints a code line */
void emitCode(char *codeLine)
{
 fprintf(code, "%s\n", codeLine);
}

/* Procedure emitRegImm prints a code line
 * that takes one register and one immidiate
 */
void emitRegImm(char *op, char *reg, int imm)
{
  fprintf(code, "%s %s %d\n", op, reg, imm);
}

/* Procedure emitRegAddr prints a code line
 * that takes one register and one address
 */
void emitRegAddr(char *op, char *reg1, int offset, char *reg2)
{
  fprintf(code, "%s %s ", op, reg1);
  if(offset)
    fprintf(code, "%d", offset);
  fprintf(code, "(%s)\n", reg2);
}

/* Procedure emitRegAddr prints a code line
 * that takes two register and one immidiate
 */
void emitRegRegImm(char *op, char *reg1, char *reg2, int imm)
{
  fprintf(code, "%s %s %s %d\n", op, reg1, reg2, imm);
}

/* Procedure emitRegRegReg prints a code line
 * that takes one register
 */
void emitReg(char *op, char *reg)
{
  fprintf(code, "%s %s\n", op, reg);
}

/* Procedure emitRegRegReg prints a code line
 * that takes two registers
 */
void emitRegReg(char *op, char *reg1, char *reg2)
{
  fprintf(code, "%s %s %s\n", op, reg1, reg2);
}

/* Procedure emitRegRegReg prints a code line
 * that takes three registers
 */
void emitRegRegReg(char *op, char *reg1, char *reg2, char *reg3)
{
  fprintf(code, "%s %s %s %s\n", op, reg1, reg2, reg3);
}