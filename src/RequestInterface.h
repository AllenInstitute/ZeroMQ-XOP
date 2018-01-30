#pragma once

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class RequestInterface
{
public:
  explicit RequestInterface(std::string identity, std::string payload);
  explicit RequestInterface(std::string payload);
  void CanBeProcessed() const;
  json Call() const;

  std::string GetCallerIdentity() const;
  bool HasValidMessageId() const;
  std::string GetMessageId() const;

private:
  friend std::ostream &operator<<(std::ostream &out, RequestInterface req);
  void FillFromJSON(json j);

  int m_version;
  std::string m_callerIdentity, m_messageId;
  CallFunctionOperationPtr m_op;
};

std::ostream &operator<<(std::ostream &out, RequestInterface req);
