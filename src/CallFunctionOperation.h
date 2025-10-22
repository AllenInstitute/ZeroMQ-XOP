#pragma once

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class CallFunctionOperation
{
public:
  explicit CallFunctionOperation(json j);
  void CanBeProcessed() const;
  json Call();

  friend struct fmt::formatter<CallFunctionOperation>;

  // Return the Igor history outputted during the function call
  std::string GetHistoryDuringCall() const;

private:
  std::string m_name;
  std::vector<std::string> m_params;
  std::string m_historyDuringCall;
  json m_json;
};

template <>
struct fmt::formatter<CallFunctionOperation> : fmt::formatter<std::string>
{
  // parse is inherited from formatter<std::string>.
  template <typename FormatContext>
  auto format(const CallFunctionOperation &op, FormatContext &ctx) const
  {
    return format_to(ctx.out(), "name={}, params={}", op.m_name, op.m_params);
  }
};
