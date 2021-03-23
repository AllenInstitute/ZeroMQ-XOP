#pragma once

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class CallFunctionOperation
{
public:
  explicit CallFunctionOperation(json j);
  void CanBeProcessed() const;
  json Call() const;

  friend struct fmt::formatter<CallFunctionOperation>;

private:
  std::string m_name;
  std::vector<std::string> m_params;
};

template <>
struct fmt::formatter<CallFunctionOperation> : fmt::formatter<std::string>
{
  // parse is inherited from formatter<std::string>.
  template <typename FormatContext>
  auto format(CallFunctionOperation op, FormatContext &ctx)
  {
    return format_to(ctx.out(), "name={}, params={}", op.m_name, op.m_params);
  }
};
