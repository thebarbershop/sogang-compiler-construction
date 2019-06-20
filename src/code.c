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
#include "util.h"

/* Procedure emitComment prints a comment line 
 * with comment c in the code file
 */
void emitComment(const char *c)
{
  if (TraceCode && c && c[0])
    fprintf(code, "# %s\n", c);
}

/* Procedure emitCode prints a code line */
void emitCode(const char *codeLine)
{
  fprintf(code, "%s\n", codeLine);
}

/* Procedure emitRegImm prints a code line
 * that takes one register and one immidiate
 */
void emitRegImm(const char *op, const char *reg, int imm)
{
  fprintf(code, "%s %s %d\n", op, reg, imm);
}

/* Procedure emitRegAddr prints a code line
 * that takes one register and one address
 */
void emitRegAddr(const char *op, const char *reg1, const char *symbol, int imm, const char *reg2)
{
  fprintf(code, "%s %s ", op, reg1);
  if (!symbol && !imm && reg2)
    fprintf(code, "(%s)\n", reg2);
  else if (!symbol && imm && !reg2)
    fprintf(code, "%d\n", imm);
  else if (!symbol && imm && reg2)
    fprintf(code, "%d(%s)\n", imm, reg2);
  else if (symbol && !imm && !reg2)
    fprintf(code, "%s\n", symbol);
  else if (symbol && imm && !reg2)
    fprintf(code, "%s+%d\n", symbol, imm);
  else if (symbol && imm && reg2)
    fprintf(code, "%s+%d(%s)\n", symbol, imm, reg2);
}

/* Procedure emitRegAddr prints a code line
 * that takes two register and one immidiate
 */
void emitRegRegImm(const char *op, const char *reg1, const char *reg2, int imm)
{
  fprintf(code, "%s %s %s %d\n", op, reg1, reg2, imm);
}

/* Procedure emitRegRegReg prints a code line
 * that takes one register
 */
void emitReg(const char *op, const char *reg)
{
  fprintf(code, "%s %s\n", op, reg);
}

/* Procedure emitRegRegReg prints a code line
 * that takes two registers */
void emitRegReg(const char *op, const char *reg1, const char *reg2)
{
  fprintf(code, "%s %s %s\n", op, reg1, reg2);
}

/* Procedure emitRegRegReg prints a code line
 * that takes three registers */
void emitRegRegReg(const char *op, const char *reg1, const char *reg2, const char *reg3)
{
  fprintf(code, "%s %s %s %s\n", op, reg1, reg2, reg3);
}

/* Procedure emitLabel prints a code line
 * that takes one label number */
void emitLabel(const char *op, int label)
{
  fprintf(code, "%s L%d\n", op, label);
}

/* Procedure emitRegLabel prints a code line
 * that takes one register and one label */
void emitRegLabel(const char *op, const char *reg, int label)
{
  fprintf(code, "%s %s L%d\n", op, reg, label);
}

/* Procedure emitLabel prints a code line
 * that indicates a label */
void emitLabelNum(int label)
{
  fprintf(code, "L%d:\n", label);
}

/* Procedure emitLabel prints a code line
 * that indicates a symbol */
void emitLabelStr(const char *symbol)
{
  fprintf(code, "%s:\n", symbol);
}