#include "ZeroMQ.h"
#include "MessageHandler.h"
#include "RequestInterface.h"

#include <chrono>
#include <thread>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

using namespace std::chrono_literals;
std::recursive_mutex threadMutex;
ConcurrentQueue<RequestInterfacePtr> reqQueue;
bool threadShouldFinish;
std::recursive_mutex threadShouldFinishMutex;

/// A reply queued by CallAndReply() (main thread) for WorkerThread to actually
/// send.
struct OutgoingReply
{
  std::string identity;
  std::string message;
};

// Replies are queued here instead of being sent directly from CallAndReply()
// (main thread) so that WorkerThread is the only thread that ever calls
// ZeroMQServerSend/ZeroMQServerReceive, i.e. the only thread that ever locks
// GetMutex(SocketTypes::Server).
ConcurrentQueue<OutgoingReply> outgoingQueue;

/// Sends every reply queued by CallAndReply() so far. Must only be called
/// from WorkerThread
void DrainOutgoingReplies()
{
  outgoingQueue.apply_to_all(
      [](const OutgoingReply &reply)
      {
        try
        {
          auto rc = ZeroMQServerSend(reply.identity, reply.message);

          DEBUG_OUTPUT("ZeroMQServerSend returned {}", rc);
        }
        catch(const std::exception &e)
        {
          EMERGENCY_OUTPUT(
              "Caught std::exception with what=\"{}\". This must NOT happen!",
              e.what());
        }
        catch(...)
        {
          EMERGENCY_OUTPUT("Caught exception. This must NOT happen!");
        }
      });
}

void WorkerThread()
{
  DEBUG_OUTPUT("Begin");

  zmq_msg_t identityMsg;
  zmq_msg_t payloadMsg;

  int rc = zmq_msg_init(&identityMsg);
  ZEROMQ_ASSERT(rc == 0);

  rc = zmq_msg_init(&payloadMsg);
  ZEROMQ_ASSERT(rc == 0);

  for(;;)
  {
    try
    {
      // check if stop is requested
      {
        LockGuard lock(threadShouldFinishMutex);
        if(threadShouldFinish)
        {
          DEBUG_OUTPUT("Exiting");
          break;
        }
      }

      DrainOutgoingReplies();

      auto numBytes = ZeroMQServerReceive(&identityMsg, &payloadMsg);

      if(numBytes == -1 && zmq_errno() == EAGAIN) // timeout
      {
        continue;
      }

      ZEROMQ_ASSERT(numBytes >= 0);

      DEBUG_OUTPUT("numBytes={}", numBytes);

      const auto identity = CreateStringFromZMsg(&identityMsg);

      try
      {
        try
        {
          const auto payload = CreateStringFromZMsg(&payloadMsg);
          auto req           = RequestInterface::Create(identity, payload);
          reqQueue.push(req);
        }
        catch(const std::bad_alloc &)
        {
          throw RequestInterfaceException(REQ_OUT_OF_MEMORY);
        }
      }
      catch(const IgorException &e)
      {
        const json reply = e;
        rc = ZeroMQServerSend(identity, reply.dump(DEFAULT_INDENT));

        DEBUG_OUTPUT("ZeroMQSendAsServer returned {}", rc);
      }
    }
    catch(const std::exception &e)
    {
      EMERGENCY_OUTPUT(
          "Caught std::exception with what = \"{}\". This must NOT happen!",
          e.what());
    }
    catch(...)
    {
      EMERGENCY_OUTPUT("Caught exception. This must NOT happen!");
    }
  }

  // ignore errors
  zmq_msg_close(&identityMsg);
  zmq_msg_close(&payloadMsg);
}

void CallAndReply(const RequestInterfacePtr &req) noexcept
{
  try
  {
    try
    {
      auto doc     = CallIgorFunctionFromReqInterface(req);
      auto message = doc.dump(DEFAULT_INDENT);

      // Queued rather than sent directly -- see outgoingQueue's comment.
      outgoingQueue.push({req->GetCallerIdentity(), message});
    }
    catch(const std::exception &e)
    {
      EMERGENCY_OUTPUT(
          "Caught std::exception with what=\"{}\". This must NOT happen!",
          e.what());
    }
  }
  catch(...)
  {
    EMERGENCY_OUTPUT("Caught exception. This must NOT happen!");
  }
}

} // anonymous namespace

void MessageHandler::Start()
{
  LockGuard lock(threadMutex);

  if(m_thread.joinable())
  {
    throw IgorException(HANDLER_ALREADY_RUNNING);
  }

  DEBUG_OUTPUT("Trying to start the handler.");

  if(!GlobalData::Instance().HasBindsOrConnections(SocketTypes::Server))
  {
    throw IgorException(HANDLER_NO_CONNECTION);
  }

  // Reset the stop flag *before* the worker thread is created, while still
  // holding threadMutex. Fixes issue #76
  {
    LockGuard innerLock(threadShouldFinishMutex);
    threadShouldFinish = false;
  }

  DEBUG_OUTPUT("Before WorkerThread() start.");

  auto t = std::thread(WorkerThread);
  m_thread.swap(t);
}

void MessageHandler::Stop()
{
  LockGuard lock(threadMutex);

  if(!m_thread.joinable())
  {
    return;
  }

  DEBUG_OUTPUT("Shutting down the handler.");

  {
    LockGuard innerLock(threadShouldFinishMutex);
    threadShouldFinish = true;
  }

  m_thread.join();

  // WorkerThread might have exited between its last DrainOutgoingReplies()
  // call and a reply queued right afterwards by CallAndReply()
  // to flush any such leftover reply here
  DrainOutgoingReplies();
}

bool MessageHandler::IsRunning() const
{
  LockGuard lock(threadMutex);

  return m_thread.joinable();
}

void MessageHandler::HandleAllQueuedMessages()
{
  if(!RunningInMainThread() || reqQueue.empty())
  {
    return;
  }

  auto msg = fmt::format("IDLE event messages: #{}", reqQueue.size());
  GlobalData::Instance().AddLogEntry(msg);

  reqQueue.apply_to_all(CallAndReply);
}

MessageHandler::~MessageHandler()
{
  Stop();
}
