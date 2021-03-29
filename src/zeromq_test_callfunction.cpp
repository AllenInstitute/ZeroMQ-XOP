#include "ZeroMQ.h"
#include "CallFunctionOperation.h"
#include "RequestInterface.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// string zeromq_test_callfunction(string msg)
extern "C" int zeromq_test_callfunction(zeromq_test_callfunctionParams *p)
{
  BEGIN_OUTER_CATCH

  auto msg = GetStringFromHandle(p->msg);
  WMDisposeHandle(p->msg);

  DEBUG_OUTPUT("input={}", msg);

  auto doc = CallIgorFunctionFromMessage(msg);

  auto retMessage = doc.dump(DEFAULT_INDENT);
  auto len        = retMessage.size();

  DEBUG_OUTPUT("len={}, retMessage={:.255s}", len, retMessage);

  p->result = WMNewHandle(len);
  ASSERT(p->result != nullptr);
  memcpy(*(p->result), retMessage.c_str(), len);

  END_OUTER_CATCH
}
