%{
  #include <common/assembler_common.hpp>

  #include <iostream>

  using namespace common;

  extern char* yytext;
  extern int yylex();
  extern int yyparse();
  void yyerror(const char *s);
%}

%defines "misc/parser.hpp"
%output "misc/parser.cpp"

%code requires 
{
  #include <cstdint>
}

%union
{
  uint32_t number;
  char* string;
  uint8_t character;
}

%token <number> LITERAL
%token <string> SYMBOL
%token <character> GPRX CSRX

%token GLOBAL EXTERN SECTION WORD SKIP END /* Direktive */
%token HALT INT IRET CALL RET
%token JMP BEQ BNE BGT /* Skokovi */
%token PUSH POP /* Stack */
%token XCHG ADD SUB MUL DIV /* Aritmeticke operacije */
%token NOT AND OR XOR SHL SHR /* Logicke operacije */
%token LD ST CSRRD CSRWR /* Operacije sa registrima */
%token ENDL
%token LBRACK RBRACK COLON COMMA DOLLAR PLUS /* Specijalni znakovi */

%start program

%%

program:  lines         { AssemblerCommon::assembler->endAssembly(); }
          |
          /* EPSILON */ { AssemblerCommon::assembler->endAssembly(); }
          ;

lines:  line
        |
        lines line
        ;

line:   label statement ENDL  { ++AssemblerCommon::currentSourceFileLine; }
        |
        statement ENDL        { ++AssemblerCommon::currentSourceFileLine; }
        ;

label:  SYMBOL COLON
        ;

statement:  instruction
            |
            directive
            ;

instruction:  HALT
              |
              INT
              |
              IRET
              |
              CALL LITERAL
              |
              RET
              |
              JMP initializator
              |
              BEQ GPRX COMMA GPRX COMMA initializator
              |
              BNE GPRX COMMA GPRX COMMA initializator
              |
              BGT GPRX COMMA GPRX COMMA initializator
              |
              PUSH GPRX
              |
              POP GPRX
              |
              XCHG GPRX COMMA GPRX
              |
              ADD GPRX COMMA GPRX
              |
              SUB GPRX COMMA GPRX
              |
              MUL GPRX COMMA GPRX
              |
              DIV GPRX COMMA GPRX
              |
              NOT GPRX
              |
              AND GPRX COMMA GPRX
              |
              OR GPRX COMMA GPRX
              |
              XOR GPRX COMMA GPRX
              |
              SHL GPRX COMMA GPRX
              |
              SHR GPRX COMMA GPRX
              |
              LD mem_operand COMMA GPRX
              |
              ST GPRX COMMA mem_operand
              |
              CSRRD CSRX COMMA GPRX
              |
              CSRWR GPRX COMMA CSRX
              ;

mem_operand:  DOLLAR initializator
              |
              initializator
              |
              GPRX
              |
              LBRACK GPRX RBRACK
              |
              LBRACK GPRX PLUS initializator RBRACK
              ;

directive:  GLOBAL global_symbol_list
            |
            EXTERN extern_symbol_list
            |
            SECTION SYMBOL
            |
            WORD initializator_list
            |
            SKIP LITERAL
            |
            END { AssemblerCommon::assembler->endAssembly(); YYACCEPT; }
            ;



global_symbol_list: SYMBOL                              { AssemblerCommon::assembler->insertGlobalSymbol($1); }
                    |
                    global_symbol_list COMMA SYMBOL     { AssemblerCommon::assembler->insertGlobalSymbol($3); }
                    ;

extern_symbol_list: SYMBOL
                    |
                    extern_symbol_list COMMA SYMBOL
                    ;

initializator_list: initializator
                    |
                    initializator_list initializator
                    ;


initializator:  SYMBOL
                |
                LITERAL
                ;


%%

void yyerror(const char* message)
{
   uint32_t lineNum = AssemblerCommon::currentSourceFileLine;
   std::cout << "Parserska greska na liniji " << lineNum << "! Greska na simbolu: " << yytext;
}