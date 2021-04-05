
#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_sub_connect(string remotePoint)
extern "C" int zeromq_sub_connect(zeromq_sub_connectParams *p)
{
  BEGIN_OUTER_CATCH

  DoBindOrConnect(p->remotePoint, SocketTypes::Subscriber);

  END_OUTER_CATCH
}
