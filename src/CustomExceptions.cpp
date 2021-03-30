#include "ZeroMQ.h"
#include "CustomExceptions.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

//--------------------------------------------------------------
// IgorException
//--------------------------------------------------------------

IgorException::IgorException(int errorCode) : IgorException(errorCode, "")
{
}

IgorException::IgorException(int errorCode, const std::string &errorMessage)
    : m_errorCode(errorCode), m_message(errorMessage)
{
}

IgorException::~IgorException()
{
  // do nothing
}

const char *IgorException::what() const noexcept
{
  return m_message.what();
}

int IgorException::GetErrorCode() const
{
  return m_errorCode;
}

int IgorException::HandleException() const
{
  NORMAL_OUTPUT("{}", what());

  return m_errorCode;
}

// Allow to serialize IgorExceptions to JSON
void to_json(json &j, const IgorException &e)
{
  j["errorCode"] = json{{"value", e.GetErrorCode()}, {"msg", e.what()}};
}

//--------------------------------------------------------------
// std::exception
//--------------------------------------------------------------

int HandleException(const std::exception &e)
{
  EMERGENCY_OUTPUT(
      "Encountered unhandled C++ exception during XOP execution: {}", e.what());

  return UNHANDLED_CPP_EXCEPTION;
}
