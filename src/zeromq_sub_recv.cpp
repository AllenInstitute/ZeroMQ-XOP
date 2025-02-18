#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// string zeromq_sub_recv()
extern "C" int zeromq_sub_recv(zeromq_sub_recvParams *p)
{
  BEGIN_OUTER_CATCH

  auto wait = GlobalData::Instance().GetRecvBusyWaitingFlag();

  for(;;)
  {
    ZeroMQMessageSharedPtrVec vec;
    int ret = ZeroMQSubscriberReceive(vec, false);

    if(ret == -1 && zmq_errno() == EAGAIN) // timeout
    {
      if(!wait || SpinProcess()) // user requested abort or we should not wait
      {
        InitHandle(&(p->result), 0);
        InitHandle(p->filter, 0);
        break;
      }

      if(RunningInMainThread())
      {
        XOPSilentCommand("DoXOPIdle");
      }

      continue;
    }

    ZEROMQ_ASSERT(ret == 0);

    auto filterMsg  = vec[0]->get();
    auto payloadMsg = vec[1]->get();

    WriteZMsgIntoHandle(&(p->result), payloadMsg);
    WriteZMsgIntoHandle(p->filter, filterMsg);

    auto msg    = CreateStringFromZMsg(payloadMsg);
    auto filter = CreateStringFromZMsg(filterMsg);

    GlobalData::Instance().AddLogEntry(filter + ":" + msg,
                                       MessageDirection::Incoming);

    DEBUG_OUTPUT("ret={}", ret);
    break;
  }

  END_OUTER_CATCH
}
