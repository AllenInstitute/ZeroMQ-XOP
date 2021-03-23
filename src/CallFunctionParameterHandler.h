#pragma once

#include "ZeroMQ.h"
#include "IgorTypeUnion.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class CallFunctionParameterHandler
{
public:
  CallFunctionParameterHandler(StringVector params, FunctionInfo fip);
  ~CallFunctionParameterHandler();

  // Return a jsons style array for the pass-by-reference parameters
  json GetPassByRefInputArray();
  json GetReturnValues();
  void *GetReturnValueStorage();
  unsigned char *GetParameterValueStorage();

private:
  json ReadPassByRefParameters(int first, int last);
  unsigned char m_values[MAX_NUM_PARAMS * sizeof(double)] = {};
  std::vector<int> m_paramTypes;
  std::vector<CountInt> m_paramSizesInBytes;
  int m_numInputParams;
  int m_returnType;
  IgorTypeUnion m_retStorage = {};
};
