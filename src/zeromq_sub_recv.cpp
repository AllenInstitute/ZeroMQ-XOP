#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// string zeromq_sub_recv()
extern "C" int zeromq_sub_recv(zeromq_sub_recvParams *p)
{
  BEGIN_OUTER_CATCH

  zmq_msg_t filterMsg;
  int rc = zmq_msg_init(&filterMsg);
  ZEROMQ_ASSERT(rc == 0);

  zmq_msg_t payloadMsg;
  rc = zmq_msg_init(&payloadMsg);
  ZEROMQ_ASSERT(rc == 0);

  auto wait = GlobalData::Instance().GetRecvBusyWaitingFlag();

  for(;;)
  {
    int numBytes = ZeroMQSubscriberReceive(&filterMsg, &payloadMsg);

    if(numBytes == -1 && zmq_errno() == EAGAIN) // timeout
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

    ZEROMQ_ASSERT(numBytes >= 0);

    WriteZMsgIntoHandle(&(p->result), &payloadMsg);
    WriteZMsgIntoHandle(p->filter, &filterMsg);

    auto msg    = CreateStringFromZMsg(&filterMsg);
    auto filter = CreateStringFromZMsg(&payloadMsg);

    GlobalData::Instance().AddLogEntry(filter + ":" + msg,
                                       MessageDirection::Incoming);

    DEBUG_OUTPUT("numBytes={}", numBytes);
    break;
  }

  rc = zmq_msg_close(&payloadMsg);
  ZEROMQ_ASSERT(rc == 0);

  rc = zmq_msg_close(&filterMsg);
  ZEROMQ_ASSERT(rc == 0);

  END_OUTER_CATCH
}
