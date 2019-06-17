/****************************************************/
/* File: code.h                                     */
/* Code emitting utilities for the C- compiler      */
/* and interface to the SPIM machine                */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

#ifndef _CODE_H_
#define _CODE_H_

/* Procedure emitComment prints a comment line 
 * with comment c in the code file
 */
void emitComment(char *c);

/* Procedure emitCode prints a code line */
void emitCode(char *code);

/* Procedure emitRegImm prints a code line
 * that takes one register and one immidiate
 */
void emitRegImm(char *op, char *reg, int imm);

/* Procedure emitRegAddr prints a code line
 * that takes one register and one address
 */
void emitRegAddr(char *op, char *reg1, int offset, char *reg2);

/* Procedure emitRegAddr prints a code line
 * that takes two registers and one immidiate
 */
void emitRegRegImm(char *op, char *reg1, char *reg2, int imm);

/* Procedure emitRegRegReg prints a code line
 * that takes one register
 */
void emitReg(char *op, char *reg);

/* Procedure emitRegRegReg prints a code line
 * that takes two registers
 */
void emitRegReg(char *op, char *reg1, char *reg2);

/* Procedure emitRegRegReg prints a code line
 * that takes three registers
 */
void emitRegRegReg(char *op, char *reg1, char *reg2, char *reg3);
#endif
