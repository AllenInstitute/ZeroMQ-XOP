#include "ZeroMQ.h"
#include <string>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_set_interceptor_func(string funcName)
extern "C" int zeromq_set_interceptor_func(zeromq_set_interceptor_funcParams *p)
{
  BEGIN_OUTER_CATCH

  const auto funcName = GetStringFromHandleWithDispose(p->funcName);

  DEBUG_OUTPUT("funcName = {}", funcName);
  GlobalData::Instance().SetInterceptorFunctionName(funcName);

  END_OUTER_CATCH
}
