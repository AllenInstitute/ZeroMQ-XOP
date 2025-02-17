#include "ZeroMQ.h"
#include "send_struct.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_pub_send_multi(WAVEWAVE payload)
extern "C" int zeromq_pub_send_multi(zeromq_pub_send_multiParams *p)
{
  BEGIN_OUTER_CATCH

  const auto sendStorage = GatherPubData(p->payload);

  int rc = ZeroMQPublisherSend(sendStorage);
  ASSERT(rc >= 0);

  END_OUTER_CATCH
}
