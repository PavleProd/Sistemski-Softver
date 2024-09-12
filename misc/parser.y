%{
  #include <assembler/assembler_common.hpp>
  #include <common/assembler_common_structures.hpp>

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
          ;

lines:  line
        |
        lines line
        ;

line:   label statement ENDL  { ++AssemblerCommon::currentSourceFileLine; }
        |
        statement ENDL        { ++AssemblerCommon::currentSourceFileLine; }
        ;

label:  SYMBOL COLON { AssemblerCommon::assembler->defineSymbol($1); }
        ;

statement:  instruction
            |
            directive
            |
            /* EPSILON */
            ;

instruction:  HALT  { AssemblerCommon::assembler->insertInstruction(InstructionTypes::HALT, {}); }
              |
              INT { AssemblerCommon::assembler->insertInstruction(InstructionTypes::INT, {});}
              |
              IRET { AssemblerCommon::assembler->insertInstruction(InstructionTypes::IRET, {});}
              |
              jumps
              |
              RET { AssemblerCommon::assembler->insertInstruction(InstructionTypes::RET, {});}
              |
              PUSH GPRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::PUSH, {$2}); }
              |
              POP GPRX  { AssemblerCommon::assembler->insertInstruction(InstructionTypes::POP, {$2}); }
              |
              XCHG GPRX COMMA GPRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::XCHG, {$2, $4}); }
              |
              ADD GPRX COMMA GPRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::ADD, {$2, $4}); }
              |
              SUB GPRX COMMA GPRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::SUB, {$2, $4}); }
              |
              MUL GPRX COMMA GPRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::MUL, {$2, $4}); }
              |
              DIV GPRX COMMA GPRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::DIV, {$2, $4}); }
              |
              NOT GPRX            { AssemblerCommon::assembler->insertInstruction(InstructionTypes::NOT, {$2}); }
              |
              AND GPRX COMMA GPRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::AND, {$2, $4}); }
              |
              OR GPRX COMMA GPRX  { AssemblerCommon::assembler->insertInstruction(InstructionTypes::OR, {$2, $4}); }
              |
              XOR GPRX COMMA GPRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::XOR, {$2, $4}); }
              |
              SHL GPRX COMMA GPRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::SHL, {$2, $4}); }
              |
              SHR GPRX COMMA GPRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::SHR, {$2, $4}); }
              |
              load
              |
              store
              |
              CSRRD CSRX COMMA GPRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::CSRRD, {$2, $4}); }
              |
              CSRWR GPRX COMMA CSRX { AssemblerCommon::assembler->insertInstruction(InstructionTypes::CSRWR, {$2, $4}); }
              ;

jumps:  CALL LITERAL  { AssemblerCommon::assembler->insertJumpInstructionLiteral(InstructionTypes::CALL, {$2}); }
        |
        CALL SYMBOL { AssemblerCommon::assembler->insertJumpInstructionSymbol(InstructionTypes::CALL, {$2}); }
        |
        JMP LITERAL { AssemblerCommon::assembler->insertJumpInstructionLiteral(InstructionTypes::JMP, {$2}); }
        |
        JMP SYMBOL { AssemblerCommon::assembler->insertJumpInstructionSymbol(InstructionTypes::JMP, {$2}); }
        |
        BEQ GPRX COMMA GPRX COMMA SYMBOL { AssemblerCommon::assembler->insertJumpInstructionSymbol(InstructionTypes::BEQ, {$2, $4, $6}); }
        |
        BEQ GPRX COMMA GPRX COMMA LITERAL { AssemblerCommon::assembler->insertJumpInstructionLiteral(InstructionTypes::BEQ, {$2, $4, $6}); }
        |
        BNE GPRX COMMA GPRX COMMA SYMBOL { AssemblerCommon::assembler->insertJumpInstructionSymbol(InstructionTypes::BNE, {$2, $4, $6}); }
        |
        BNE GPRX COMMA GPRX COMMA LITERAL { AssemblerCommon::assembler->insertJumpInstructionLiteral(InstructionTypes::BNE, {$2, $4, $6}); }
        |
        BGT GPRX COMMA GPRX COMMA SYMBOL { AssemblerCommon::assembler->insertJumpInstructionSymbol(InstructionTypes::BGT, {$2, $4, $6}); }
        |
        BGT GPRX COMMA GPRX COMMA LITERAL { AssemblerCommon::assembler->insertJumpInstructionLiteral(InstructionTypes::BGT, {$2, $4, $6}); }
        ;

load: LD DOLLAR SYMBOL COMMA GPRX { AssemblerCommon::assembler->insertLoadInstructionSymbol(MemoryInstructionType::SYM_IMM, {$3, $5}); }
      |
      LD DOLLAR LITERAL COMMA GPRX { AssemblerCommon::assembler->insertLoadInstructionLiteral(MemoryInstructionType::LIT_IMM, {$3, $5}); }
      |
      LD SYMBOL COMMA GPRX { AssemblerCommon::assembler->insertLoadInstructionSymbol(MemoryInstructionType::SYM_MEM_DIR, {$2, $4}); }
      |
      LD LITERAL COMMA GPRX { AssemblerCommon::assembler->insertLoadInstructionLiteral(MemoryInstructionType::LIT_MEM_DIR, {$2, $4}); }
      |
      LD GPRX COMMA GPRX  { AssemblerCommon::assembler->insertLoadInstructionRegister(MemoryInstructionType::REG_IMM, {$2, $4}); }
      |
      LD LBRACK GPRX RBRACK COMMA GPRX { AssemblerCommon::assembler->insertLoadInstructionRegister(MemoryInstructionType::REG_MEM_DIR, {$3, $6}); }
      |
      LD LBRACK GPRX PLUS SYMBOL RBRACK COMMA GPRX  /* NE MOZE DA SE DESI */
      |
      LD LBRACK GPRX PLUS LITERAL RBRACK COMMA GPRX { AssemblerCommon::assembler->insertLoadInstructionLiteral(MemoryInstructionType::REG_REL_LIT, {$3, $5, $8}); }
      ;

store:  ST GPRX COMMA SYMBOL { AssemblerCommon::assembler->insertStoreInstructionSymbol(MemoryInstructionType::SYM_MEM_DIR, {$2, $4}); }
        |
        ST GPRX COMMA LITERAL { AssemblerCommon::assembler->insertStoreInstructionLiteral(MemoryInstructionType::LIT_MEM_DIR, {$2, $4}); }
        |
        ST GPRX COMMA GPRX { AssemblerCommon::assembler->insertStoreInstructionRegister(MemoryInstructionType::REG_IMM, {$2, $4}); }
        |
        ST GPRX COMMA LBRACK GPRX RBRACK { AssemblerCommon::assembler->insertStoreInstructionRegister(MemoryInstructionType::REG_MEM_DIR, {$2, $5}); }
        |
        ST GPRX COMMA LBRACK GPRX PLUS SYMBOL RBRACK /* NE MOZE DA SE DESI */
        |
        ST GPRX COMMA LBRACK GPRX PLUS LITERAL RBRACK { AssemblerCommon::assembler->insertStoreInstructionLiteral(MemoryInstructionType::REG_REL_LIT, {$2, $5, $7}); }
        ;

directive:  GLOBAL global_symbol_list
            |
            EXTERN extern_symbol_list
            |
            SECTION SYMBOL  { AssemblerCommon::assembler->openNewSection($2); }
            |
            WORD initializator_list
            |
            SKIP LITERAL    { AssemblerCommon::assembler->insertBSS($2); }
            |
            END { AssemblerCommon::assembler->endAssembly(); YYACCEPT; }
            ;



global_symbol_list: SYMBOL { AssemblerCommon::assembler->insertGlobalSymbol($1); }
                    |
                    global_symbol_list COMMA SYMBOL { AssemblerCommon::assembler->insertGlobalSymbol($3); }
                    ;

extern_symbol_list: SYMBOL { AssemblerCommon::assembler->insertExternSymbol($1); }
                    |
                    extern_symbol_list COMMA SYMBOL { AssemblerCommon::assembler->insertExternSymbol($3); }
                    ;

initializator_list: initializator
                    |
                    initializator_list COMMA initializator
                    ;


initializator:  SYMBOL  { AssemblerCommon::assembler->insertSymbol($1); }
                |
                LITERAL { AssemblerCommon::assembler->insertLiteral($1); }
                ;


%%

void yyerror(const char* message)
{
  uint32_t lineNum = AssemblerCommon::currentSourceFileLine;
  std::cout << "Parserska greska na liniji " << lineNum << "! Greska na simbolu: " << yytext;
}