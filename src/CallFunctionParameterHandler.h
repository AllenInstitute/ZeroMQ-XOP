#ifndef CALL_FUNCTION_PARAMETER_HANDLER_H
#define CALL_FUNCTION_PARAMETER_HANDLER_H

#include "IgorTypeUnion.h"
#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class CallFunctionParameterHandler
{
public:
  CallFunctionParameterHandler(StringVector params, FunctionInfo fip);
  ~CallFunctionParameterHandler();

  CallFunctionParameterHandler(const CallFunctionParameterHandler &) = delete;
  CallFunctionParameterHandler(CallFunctionParameterHandler &&)      = delete;
  CallFunctionParameterHandler &
  operator=(const CallFunctionParameterHandler &) = delete;
  CallFunctionParameterHandler &
  operator=(CallFunctionParameterHandler &&) = delete;

  // Return a jsons style array for the pass-by-reference parameters
  json GetPassByRefInputArray();
  json GetReturnValues();
  void *GetReturnValueStorage();
  unsigned char *GetParameterValueStorage();

private:
  json ReadPassByRefParameters(int first, int last);
  std::array<unsigned char, MAX_NUM_PARAMS * sizeof(double)> m_values{};
  std::vector<int> m_paramTypes;
  std::vector<CountInt> m_paramSizesInBytes;
  bool m_multipleReturnValueSyntax;
  int m_numReturnValues;
  int m_numInputParams;
  int m_returnType;
  IgorTypeUnion m_retStorage = {};
};

#endif // CALL_FUNCTION_PARAMETER_HANDLER_H
