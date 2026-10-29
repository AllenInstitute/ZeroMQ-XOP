#include "ZeroMQ.h"

#include <cmath>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_client_set_recv_timeout(variable timeout)
extern "C" int
zeromq_client_set_recv_timeout(zeromq_client_set_recv_timeoutParams *p)
{
  BEGIN_OUTER_CATCH

  DEBUG_OUTPUT("timeout = {}", p->timeout);

  if(!std::isfinite(p->timeout) || p->timeout < 0)
  {
    throw IgorException(
        INVALID_ARG,
        fmt::format("The timeout {} must be zero or positive.", p->timeout));
  }

  GlobalData::Instance().SetClientRecvTimeout(p->timeout);

  END_OUTER_CATCH
}
