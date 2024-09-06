#pragma once

#include <common/assembler_common.hpp>

#include <array>
#include <exception>
#include <string>

namespace common
{

enum ErrorCode
{
  GLOBAL_EXTERN_CONFLICT
};

const static std::array<std::string, 3> errorMessages =
{
  {"Simbol ne moze da bude i eksterni i globalni!"},
};

class AssemblerError : public std::exception 
{
public:
  AssemblerError(ErrorCode errorCode) : errorCode(errorCode) {}

  const char* what() const noexcept override
  {
    static std::string message;
    message = std::string("Linija ") + 
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

} // namespace common

