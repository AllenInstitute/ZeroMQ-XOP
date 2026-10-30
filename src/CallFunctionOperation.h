#ifndef CALL_FUNCTION_OPERATION_H
#define CALL_FUNCTION_OPERATION_H

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class CallFunctionOperation
{
public:
  explicit CallFunctionOperation(json doc);
  void CanBeProcessed() const;
  json Call();
  void CallInterceptor(const std::string &identity, InterceptorMode mode) const;

  friend struct fmt::formatter<CallFunctionOperation>;

  // Return the Igor history outputted during the function call
  [[nodiscard]] std::string GetHistoryDuringCall() const;

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
  auto format(const CallFunctionOperation &operation, FormatContext &ctx) const
  {
    return format_to(ctx.out(), "name={}, params={}", operation.m_name,
                     operation.m_params);
  }
};

#endif // CALL_FUNCTION_OPERATION_H
