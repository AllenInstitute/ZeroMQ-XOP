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

  bool HasPassByRefParameters();

  // Return a jsons style array for the pass-by-reference parameters
  json GetPassByRefArray();
  json GetReturnValues();
  void *GetReturnValueStorage();
  unsigned char *GetParameterValueStorage();

private:
  unsigned char m_values[MAX_NUM_PARAMS * sizeof(double)];
  std::vector<int> m_paramTypes;
  std::vector<CountInt> m_paramSizesInBytes;
  bool m_hasPassByRefParams;
  int m_numInputParams;
  int m_returnType;
  IgorTypeUnion m_retStorage = {};
};
