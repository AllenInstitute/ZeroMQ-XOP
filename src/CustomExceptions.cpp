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
    : IgorException(errorCode, errorMessage, "")
{
}

IgorException::IgorException(int errorCode, const std::string &errorMessage,
                             const std::string &details)
    : m_errorCode(errorCode), m_message(errorMessage), m_details(details)
{
}

IgorException::~IgorException() = default;

const char *IgorException::what() const noexcept
{
  return m_message.what();
}

int IgorException::GetErrorCode() const
{
  return m_errorCode;
}

const char *IgorException::GetDetails() const noexcept
{
  return m_details.what();
}

int IgorException::HandleException() const
{
  if(GetDetails()[0] != '\0')
  {
    DEBUG_OUTPUT("{}", GetDetails());
  }

  NORMAL_OUTPUT("{}", what());

  return m_errorCode;
}

// Allow to serialize IgorExceptions to JSON
void to_json(json &doc, const IgorException &exception)
{
  doc["errorCode"] =
      json{{"value", exception.GetErrorCode()}, {"msg", exception.what()}};
}

//--------------------------------------------------------------
// std::exception
//--------------------------------------------------------------

int HandleException(const std::exception &exception)
{
  EMERGENCY_OUTPUT(
      "Encountered unhandled C++ exception during XOP execution: {}",
      exception.what());

  return UNHANDLED_CPP_EXCEPTION;
}
