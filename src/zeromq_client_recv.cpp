#include "ZeroMQ.h"

#include <chrono>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

/// What to do when no reply has arrived yet
enum class NoReplyAction : std::uint8_t
{
  Wait,         ///< keep waiting
  Return,       ///< return an empty string, without busy waiting
  GiveUp,       ///< the caller gives up on the reply
  ReceiveAgain, ///< abort requested, the reply might have arrived meanwhile
};

NoReplyAction GetNoReplyAction(bool wait, double timeout,
                               std::chrono::steady_clock::time_point start)
{
  if(!wait)
  {
    return NoReplyAction::Return;
  }

  const std::chrono::duration<double> elapsed =
      std::chrono::steady_clock::now() - start;

  if(timeout > 0 && elapsed.count() > timeout)
  {
    DEBUG_OUTPUT("No reply within the timeout of {} s", timeout);
    return NoReplyAction::GiveUp;
  }

  if(SpinProcess()) // abort requested
  {
    return NoReplyAction::ReceiveAgain;
  }

  if(RunningInMainThread())
  {
    XOPSilentCommand("DoXOPIdle");
  }

  return NoReplyAction::Wait;
}

void ReceiveReply(Handle *result, zmq_msg_t *payloadMsg)
{
  const auto wait    = GlobalData::Instance().GetRecvBusyWaitingFlag();
  const auto timeout = GlobalData::Instance().GetClientRecvTimeout();
  const auto start   = std::chrono::steady_clock::now();

  bool abortRequested = false;

  // the caller gives up on the reply, it must not be returned by a later call
  auto giveUp = [result]()
  {
    GlobalData::Instance().AbandonClientReplies();
    InitHandle(result, 0);
  };

  for(;;)
  {
    int numBytes = ZeroMQClientReceive(payloadMsg);

    if(numBytes == -1 && zmq_errno() == EAGAIN) // timeout
    {
      const auto action = GetNoReplyAction(wait, timeout, start);

      if(action == NoReplyAction::Wait)
      {
        continue;
      }

      if(action == NoReplyAction::Return)
      {
        InitHandle(result, 0);
        return;
      }

      if(action == NoReplyAction::GiveUp)
      {
        giveUp();
        return;
      }

      abortRequested = true;
      numBytes       = ZeroMQClientReceive(payloadMsg);

      if(numBytes == -1 && zmq_errno() == EAGAIN)
      {
        giveUp();
        return;
      }
    }

    ZEROMQ_ASSERT(numBytes >= 0);

    if(GlobalData::Instance().ClientReplyReceived())
    {
      DEBUG_OUTPUT("Discarding the reply to an abandoned request");

      if(abortRequested)
      {
        giveUp();
        return;
      }

      continue;
    }

    WriteZMsgIntoHandle(result, payloadMsg);

    auto msg = CreateStringFromZMsg(payloadMsg);
    GlobalData::Instance().AddLogEntry(msg, MessageDirection::Incoming);

    DEBUG_OUTPUT("numBytes={}", numBytes);
    return;
  }
}

} // anonymous namespace

// string zeromq_client_recv()
extern "C" int zeromq_client_recv(zeromq_client_recvParams *p)
{
  BEGIN_OUTER_CATCH

  zmq_msg_t payloadMsg;
  int rc = zmq_msg_init(&payloadMsg);
  ZEROMQ_ASSERT(rc == 0);

  ReceiveReply(&(p->result), &payloadMsg);

  rc = zmq_msg_close(&payloadMsg);
  ZEROMQ_ASSERT(rc == 0);

  END_OUTER_CATCH
}
