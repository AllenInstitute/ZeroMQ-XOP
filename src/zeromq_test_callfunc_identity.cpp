#include "ZeroMQ.h"
#include "CallFunctionOperation.h"
#include "RequestInterface.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// string zeromq_test_callfunc_identity(string msg, string identity)
extern "C" int
zeromq_test_callfunc_identity(zeromq_test_callfunc_identityParams *p)
{
  BEGIN_OUTER_CATCH

  const auto msg      = GetStringFromHandleWithDispose(p->msg);
  const auto identity = GetStringFromHandleWithDispose(p->identity);

  DEBUG_OUTPUT("input={}", msg);

  auto doc = CallIgorFunctionFromMessage(msg, identity);

  auto retMessage = doc.dump(DEFAULT_INDENT);
  auto len        = retMessage.size();

  DEBUG_OUTPUT("len={}, retMessage={:.255s}", len, retMessage);

  p->result = GetHandleFromString(retMessage);
  ASSERT(p->result != nullptr);

  END_OUTER_CATCH
}
