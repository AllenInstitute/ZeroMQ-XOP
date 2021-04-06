#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_client_connect(string remotePoint)
extern "C" int zeromq_client_connect(zeromq_client_connectParams *p)
{
  BEGIN_OUTER_CATCH

  DoBindOrConnect(p->remotePoint, SocketTypes::Client);

  END_OUTER_CATCH
}
