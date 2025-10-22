#pragma once

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class RequestInterface : public std::enable_shared_from_this<RequestInterface>
{
private:
  struct Private;

public:
  explicit RequestInterface(Private, std::string callerIdentity,
                            const std::string &payload);
  explicit RequestInterface(Private, const std::string &payload);

  static RequestInterfacePtr Create(std::string callerIdentity,
                                    const std::string &payload);
  static RequestInterfacePtr Create(const std::string &payload);
  void CanBeProcessed() const;
  void CallInterceptor() const;
  json Call() const;

  std::string GetCallerIdentity() const;
  bool HasValidMessageId() const;
  std::string GetMessageId() const;
  std::string GetHistoryDuringOperation() const;

  friend struct fmt::formatter<RequestInterface>;

private:
  struct Private
  {
    explicit Private() = default;
  };

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
    auto nicer_empty = [](const std::string &str) -> std::string
    {
      if(str.empty())
      {
        return "(not provided)";
      }

      return str;
    };

    return format_to(
        ctx.out(),
        "version={}, callerIdentity={}, messageId={}, CallFunction: {}",
        req.m_version, nicer_empty(req.m_callerIdentity),
        nicer_empty(req.m_messageId), *(req.m_op));
  }
};
