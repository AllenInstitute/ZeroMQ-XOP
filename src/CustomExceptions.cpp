#include "ZeroMQ.h"
#include "CustomExceptions.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

//--------------------------------------------------------------
// IgorException
//--------------------------------------------------------------

#ifdef WINIGOR
IgorException::IgorException() : m_errorCode(UNHANDLED_CPP_EXCEPTION)
{
}
#endif

IgorException::IgorException(int errorCode) : m_errorCode(errorCode)
{
}

IgorException::IgorException(int errorCode, std::string errorMessage)
    : m_errorCode(errorCode), m_message(std::move(errorMessage))
{
}

const char *IgorException::what() const noexcept
{
  return m_message.c_str();
}

int IgorException::HandleException() const
{
  XOPNotice_ts(what());

  return m_errorCode;
}

// Allow to serialize IgorExceptions to JSON
void to_json(json &j, const IgorException &e)
{
  j["errorCode"] = json{{"value", e.m_errorCode}, {"msg", e.what()}};
}

//--------------------------------------------------------------
// std::exception
//--------------------------------------------------------------

int HandleException(const std::exception &e)
{
  XOPNotice_ts("Encountered unhandled C++ exception during XOP execution.\r");
  XOPNotice_ts(std::string(e.what()) + CR_STR);

  return UNHANDLED_CPP_EXCEPTION;
}
