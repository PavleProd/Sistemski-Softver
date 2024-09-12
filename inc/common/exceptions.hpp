#pragma once

#include <assembler/assembler_common.hpp>

#include <array>
#include <exception>
#include <string>

namespace common
{

enum ErrorCode
{
  GLOBAL_EXTERN_CONFLICT,
  SYMBOL_REDECLARATION,
  SECTION_REDECLARATION,
  INSTRUCTION_OUTSIDE_OF_SECTION,
  UNRECOGNIZED_INSTRUCTION,
  VALUE_OVERFLOW,
  BACKPATCHING_ERROR,
  NUM_ERRORS
};

const static std::array<std::string, ErrorCode::NUM_ERRORS> errorMessages =
{
  "Simbol ne moze da bude i eksterni i globalni!",
  "Redeklaracija simbola!",
  "Redeklaracija sekcije!",
  "Instrukcija koriscena van sekcije!",
  "Instrukcija nije prepoznata od strane asemblera!",
  "Velicina operanda je prevelika za instrukciju!",
  "Greska u backpatchingu!"
};

class AssemblerError : public std::exception 
{
public:
  AssemblerError(ErrorCode errorCode) : errorCode(errorCode) {}

  const char* what() const noexcept override
  {
    static std::string message;
    message = std::string("AsemblerError, Linija ") + 
              std::to_string(AssemblerCommon::currentSourceFileLine) 
              + ": " 
              + errorMessages[errorCode];
    
    return message.c_str();
  }
private:
  ErrorCode errorCode;
};

class RuntimeError : public std::exception
{
public:
  RuntimeError(const std::string& message) : message(message) {}

  const char* what() const noexcept override
  {
    return message.c_str();
  }

private:
  std::string message;
};

class LinkerError : public std::exception
{
public:
  LinkerError(const std::string& message) : message(message) {}

  const char* what() const noexcept override
  {
    return (std::string("LinkerError: ") + message).c_str();
  }

private:
  std::string message;
};

} // namespace common

