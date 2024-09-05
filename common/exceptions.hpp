#pragma once

#include <common/common_data.hpp>

#include <array>
#include <exception>
#include <string>

namespace Common
{

enum ErrorCode
{
  GLOBAL_EXTERN_CONFLICT
};

const static std::array<std::string, 1> errorMessages =
{
  {"Simbol ne moze da bude i eksterni i globalni!"}
};

class CustomError : public std::exception 
{
public:
  CustomError(ErrorCode errorCode) : errorCode(errorCode) {}

  const char* what() const noexcept override
  {
    static std::string message;
    message = std::string("Linija ") + 
              std::to_string(currentSourceFileLine) 
              + ": " 
              + errorMessages[errorCode];
    
    return message.c_str();
  }
private:
  ErrorCode errorCode;
};

} // namespace Common

