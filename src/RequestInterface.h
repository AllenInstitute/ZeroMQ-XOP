#pragma once

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class RequestInterface
{
public:
  explicit RequestInterface(std::string callerIdentity,
                            const std::string &payload);
  explicit RequestInterface(const std::string &payload);
  void CanBeProcessed() const;
  json Call() const;

  std::string GetCallerIdentity() const;
  bool HasValidMessageId() const;
  std::string GetMessageId() const;
  std::string GetHistoryDuringOperation() const;

  friend struct fmt::formatter<RequestInterface>;

private:
  void FillFromJSON(json j);

  int m_version{};
  std::string m_callerIdentity, m_messageId;
  CallFunctionOperationPtr m_op;
};

template <>
struct fmt::formatter<RequestInterface> : fmt::formatter<std::string>
{
  // parse is inherited from formatter<std::string>.
  template <typename FormatContext>
  auto format(const RequestInterface &req, FormatContext &ctx) const
  {
    return format_to(
        ctx.out(),
        "version={}, callerIdentity={}, messageId={}, CallFunction: {}",
        req.m_version, req.m_callerIdentity,
        (req.m_messageId.empty() ? "(not provided)" : req.m_messageId),
        *(req.m_op));
  }
};
