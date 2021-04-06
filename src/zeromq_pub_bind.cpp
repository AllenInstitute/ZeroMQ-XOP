
#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_pub_bind(string localPoint)
extern "C" int zeromq_pub_bind(zeromq_pub_bindParams *p)
{
  BEGIN_OUTER_CATCH

  DoBindOrConnect(p->localPoint, SocketTypes::Publisher);

  END_OUTER_CATCH
}
