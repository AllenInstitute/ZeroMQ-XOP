#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_sub_recv_multi(WAVEWAVE payload)
extern "C" int zeromq_sub_recv_multi(zeromq_sub_recv_multiParams *p)
{
  BEGIN_OUTER_CATCH

  auto wait = GlobalData::Instance().GetRecvBusyWaitingFlag();

  ZeroMQMessageSharedPtrVec vec;

  for(;;)
  {
    int ret = ZeroMQSubscriberReceive(vec, true);

    if(ret == -1 && zmq_errno() == EAGAIN) // timeout
    {
      if(!wait || SpinProcess()) // user requested abort or we should not wait
      {
        break;
      }

      if(RunningInMainThread())
      {
        XOPSilentCommand("DoXOPIdle");
      }

      continue;
    }

    ZEROMQ_ASSERT(ret == 0);
    DEBUG_OUTPUT("ret={}", ret);

    ConvertSubData(vec, p->payload);
    break;
  }

  END_OUTER_CATCH
}
