#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// string zeromq_client_recv()
extern "C" int zeromq_client_recv(zeromq_client_recvParams *p)
{
  BEGIN_OUTER_CATCH

  zmq_msg_t payloadMsg;
  int rc = zmq_msg_init(&payloadMsg);
  ZEROMQ_ASSERT(rc == 0);

  auto wait = GlobalData::Instance().GetRecvBusyWaitingFlag();

  for(;;)
  {
    int numBytes = ZeroMQClientReceive(&payloadMsg);

    if(numBytes == -1 && zmq_errno() == EAGAIN) // timeout
    {
      if(!wait || SpinProcess()) // user requested abort or we should not wait
      {
        InitHandle(&(p->result), 0);
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

    auto msg = CreateStringFromZMsg(&payloadMsg);
    GlobalData::Instance().AddLogEntry(msg, MessageDirection::Incoming);

    DEBUG_OUTPUT("numBytes={}", numBytes);
    break;
  }

  rc = zmq_msg_close(&payloadMsg);
  ZEROMQ_ASSERT(rc == 0);

  END_OUTER_CATCH
}
