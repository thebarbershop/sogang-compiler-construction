/****************************************************/
/* File: cmin.l                                     */
/* Lex specification for C-                         */
/* Based on tiny.l source code from                 */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden, 1997                          */
/* Modified by Eom Taegyung                         */
/****************************************************/

%option yylineno
%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
%}

%x COMMENT

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t]+

%%

/* Keywords
   Matching order of Louden, 1997, page 491 */
"else"          {return ELSE;}
"if"            {return IF;}
"int"           {return INT;}
"return"        {return RETURN;}
"void"          {return VOID;}
"while"         {return WHILE;}

/* Special symbols
   Matching order of Louden, 1997, page 491 */
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"<"             {return LT;}
"<="            {return LTE;}
">"             {return GT;}
">="            {return GTE;}
"=="            {return EQ;}
"!="            {return NEQ;}
"="             {return ASSIGN;}
";"             {return SEMI;}
","             {return COMMA;}
"("             {return LPAREN;}
")"             {return RPAREN;}
"["             {return LBRACKET;}
"]"             {return RBRACKET;}
"{"             {return LBRACE;}
"}"             {return RBRACE;}


{number}        {return NUM;}
{identifier}    {return ID;}
/*{newline}       {lineno++;}*/
{whitespace}    {/* skip whitespace */}

/* To handle C Comments, use start conditions
   described in the official FLEX manual at:
   http://westes.github.io/flex/manual/Start-Conditions.html */

/* Valid comments */
"/*"            {BEGIN(COMMENT);}
<COMMENT>.      { /* eat up comment body */ }
<COMMENT>"*/"   {BEGIN(INITIAL);}

/* End of file before comment ends */
/* <COMMENT><<EOF>>{return ERROR;} */

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
/*    lineno++; */
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
/*    fprintf(listing,"\t%d: ",lineno);*/
    fprintf(listing, "\t%d: ", yylineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}