#include "CallFunctionOperation.h"
#include "RequestInterface.h"
#include "ZeroMQ.h"

#include <utility>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

const size_t MAX_MESSAGEID_LENGTH = 255;

bool IsValidMessageId(const std::string &messageId)
{
  return messageId.length() != 0 && messageId.length() <= MAX_MESSAGEID_LENGTH;
}

} // anonymous namespace

RequestInterface::RequestInterface(std::string callerIdentity,
                                   const std::string &payload)
    : m_callerIdentity(std::move(callerIdentity))
{
  json doc;
  try
  {
    doc = json::parse(payload);
  }
  catch(const std::exception &)
  {
    GlobalData::Instance().AddLogEntry(payload, m_callerIdentity,
                                       MessageDirection::Incoming);
    throw RequestInterfaceException(REQ_INVALID_JSON_OBJECT);
  }

  // the idea is to log the incoming payload as json document if possible
  GlobalData::Instance().AddLogEntry(doc, m_callerIdentity,
                                     MessageDirection::Incoming);

  DEBUG_OUTPUT("JSON Document is valid, data={}", doc.dump());
  FillFromJSON(doc);
}

RequestInterface::RequestInterface(const std::string &payload)
    : RequestInterface("", payload)
{
}

void RequestInterface::CanBeProcessed() const
{
  ASSERT(m_op);
  m_op->CanBeProcessed();
}

json RequestInterface::Call() const
{
  ASSERT(m_op);
  auto reply = m_op->Call();

  if(HasValidMessageId())
  {
    reply[MESSAGEID_KEY] = GetMessageId();
  }

  return reply;
}

std::string RequestInterface::GetCallerIdentity() const
{
  return m_callerIdentity;
}

std::string RequestInterface::GetMessageId() const
{
  return m_messageId;
}

bool RequestInterface::HasValidMessageId() const
{
  return IsValidMessageId(m_messageId);
}

std::string RequestInterface::GetHistoryDuringOperation() const
{
  ASSERT(m_op);
  return m_op->GetHistoryDuringCall();
}

void RequestInterface::FillFromJSON(json j)
{
  auto it = j.find("version");

  if(it == j.end() || !it.value().is_number_integer())
  {
    throw RequestInterfaceException(REQ_INVALID_VERSION);
  }

  auto version = it.value().get<int>();

  if(version != 1)
  {
    throw RequestInterfaceException(REQ_INVALID_VERSION);
  }

  m_version = version;

  it = j.find(MESSAGEID_KEY);

  if(it != j.end()) // messageID is optional
  {
    if(!it.value().is_string())
    {
      throw RequestInterfaceException(REQ_INVALID_MESSAGEID);
    }

    auto messageId = it.value().get<std::string>();

    if(!IsValidMessageId(messageId))
    {
      throw RequestInterfaceException(REQ_INVALID_MESSAGEID);
    }

    m_messageId = messageId;
  }

  it = j.find("CallFunction");

  if(it == j.end() || !it.value().is_object())
  {
    throw RequestInterfaceException(REQ_INVALID_OPERATION);
  }

  m_op = std::make_shared<CallFunctionOperation>(*it);

  DEBUG_OUTPUT("Request Object could be created: {}", *this);
}
