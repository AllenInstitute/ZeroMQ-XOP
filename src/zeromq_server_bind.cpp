#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_server_bind(string localPoint)
extern "C" int zeromq_server_bind(zeromq_server_bindParams *p)
{
  BEGIN_OUTER_CATCH

  DoBindOrConnect(p->localPoint, SocketTypes::Server);

  END_OUTER_CATCH
}
