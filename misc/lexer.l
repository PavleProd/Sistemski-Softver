%{
#include "parser.hpp"  

#include <cstring>
#include <cstdlib>
%}

%option outfile="lexer.cpp" header-file="lexer.hpp"

%option noyywrap

// direktive
GLOBAL    "\.global"
EXTERN    "\.extern"
SECTION   "\.section"
WORD      "\.word"
SKIP      "\.skip"
END       "\.end"

HALT      "halt"
INT       "int"
IRET      "iret"
CALL      "call"
RET       "ret"

// skokovi
JMP       "jmp"
BEQ       "beq"
BNE       "bne"
BGT       "bgt"

// stack
PUSH      "push"
POP       "pop"

// aritmeticke operacije
XCHG      "xchg"
ADD       "add"
SUB       "sub"
MUL       "mul"
DIV       "div"

// logicke operacije
NOT       "not"
AND       "and"
OR        "or"
XOR       "xor"
SHL       "shl"
SHR       "shr"

// operacije sa registrima
LD        "ld"
ST        "st"
CSRRD     "csrrd"
CSRWR     "csrwr"

// registri
GPRX      (%r[0-9])|(%r1[0-5]) /* r0-15 */
SP        "%sp" /* r14 */
PC        "%pc" /* r15 */
CSR0      "%status"
CSR1      "%handler"
CSR2      "%cause"


// specijalni znakovi
LBRACK    "\["
RBRACK    "\]"
COLON     "\:"
COMMA     "\,"
DOLLAR    "\$"
PLUS      "\+"


// regularni izrazi
LITERAL     (0[Xx])?[0-9A-Fa-f]+
SYMBOL      [A-Za-z_][A-Za-z0-9]*
COMMENT     "#"[^\n\r]*
WHITESPACE  [ \r\t]*


%%

{LITERAL} {
  yylval.number = strtol(yytext, nullptr, 0); // https://cplusplus.com/reference/cstdlib/strtol/
  return LITERAL;
}

{SYMBOL} {
  yylval.string = strdup(yytext); // https://en.cppreference.com/w/c/experimental/dynamic/strdup
  return SYMBOL;
}

{COMMENT}     { /* ignorisemo */ }
{WHITESPACE}  { /* ignorisemo */ }

\n            { return ENDL; } /* TODO: da li nam je bitna trenutna linija */

/* direktive */
{GLOBAL}  {return GLOBAL;}
{EXTERN}  {return EXTERN;}
{SECTION} {return SECTION;}
{WORD}    {return WORD;}
{SKIP}    {return SKIP;}
{END}     {return END;}

/* skokovi */
{JMP}     {return JMP;}
{BEQ}     {return BEQ;}
{BNE}     {return BNE;}
{BGT}     {return BGT;}

/* stack */
{PUSH}    {return PUSH;}
{POP}     {return POP;}

/* aritmeticke operacije */
{XCHG}    {return XCHG;}
{ADD}     {return ADD;}
{SUB}     {return SUB;}
{MUL}     {return MUL;}
{DIV}     {return DIV;}

/* logicke operacije */
{NOT}     {return NOT;}
{AND}     {return AND;}
{OR}      {return OR;}
{XOR}     {return XOR;}
{SHL}     {return SHL;}
{SHR}     {return SHR;}

/* operacije sa registrima */
{LD}      {return LD;}
{ST}      {return ST;}
{CSRRD}   {return CSRRD;}
{CSRWR}   {return CSRWR;}

/* registri */
{GPRX}    {return GPRX;}
{SP}      {return GPRX;}
{PC}      {return GPRX;}

{CSR0}    {return CSRX;}
{CSR1}    {return CSRX;}
{CSR2}    {return CSRX;}

/* specijalni znakovi */ 
{LBRACK}  {return LBRACK;}
{RBRACK}  {return RBRACK;}
{COLON}   {return COLON;}
{COMMA}   {return COMMA;}
{DOLLAR}  {return DOLLAR;}
{PLUS}    {return PLUS;}

.         { yyerror("Sintaksna greska! Simbol nije prepoznat"); }

%%