#include "ZeroMQ.h"
#include "MessageHandler.h"
#include "RequestInterface.h"

#include <atomic>
#include <chrono>
#include <thread>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

using namespace std::chrono_literals;
std::recursive_mutex threadMutex;
bool threadShouldFinish;
std::recursive_mutex threadShouldFinishMutex;

/// Incremented when the connections are closed, requests and replies of
/// an older generation belong to clients of closed connections
std::atomic<uint64_t> connectionGeneration{0};

struct IncomingRequest
{
  RequestInterfacePtr request;
  uint64_t generation;
};

ConcurrentQueue<IncomingRequest> incomingQueue;

struct OutgoingReply
{
  std::string identity;
  std::string message;
  uint64_t generation;
};

ConcurrentQueue<OutgoingReply> outgoingQueue;

void SendErrorReply(const std::string &identity, const IgorException &e)
{
  const json reply = e;

  // the client might have disconnected in the meantime
  auto rc = ZeroMQServerSend(identity, reply.dump(DEFAULT_INDENT), false);

  DEBUG_OUTPUT("ZeroMQServerSend returned {}", rc);
}

void DrainOutgoingReplies()
{
  if(RunningInMainThread() || outgoingQueue.empty())
  {
    return;
  }

  outgoingQueue.apply_to_all(
      [](const OutgoingReply &reply)
      {
        try
        {
          if(reply.generation != connectionGeneration)
          {
            DEBUG_OUTPUT("Discarding the reply to a closed connection");
            return;
          }

          // the client might have disconnected in the meantime
          auto rc = ZeroMQServerSend(reply.identity, reply.message, false);

          if(rc < 0)
          {
            DEBUG_OUTPUT("Discarding the reply to a disconnected client");
            return;
          }

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
      DrainOutgoingReplies();

      // check if stop is requested
      {
        LockGuard lock(threadShouldFinishMutex);
        if(threadShouldFinish)
        {
          DEBUG_OUTPUT("Exiting");
          break;
        }
      }

      int numBytes{};

      try
      {
        numBytes = ZeroMQServerReceive(&identityMsg, &payloadMsg);
      }
      catch(const IgorException &e)
      {
        // the message has the wrong frames, but we know the sender
        SendErrorReply(CreateStringFromZMsg(&identityMsg), e);
        continue;
      }

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
          incomingQueue.push({req, connectionGeneration});
        }
        catch(const std::bad_alloc &)
        {
          throw RequestInterfaceException(REQ_OUT_OF_MEMORY);
        }
      }
      catch(const IgorException &e)
      {
        SendErrorReply(identity, e);
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

void CallAndQueueReply(const IncomingRequest &incoming) noexcept
{
  try
  {
    try
    {
      if(incoming.generation != connectionGeneration)
      {
        DEBUG_OUTPUT("Discarding the request of a closed connection");
        return;
      }

      const auto &req = incoming.request;
      auto doc        = CallIgorFunctionFromReqInterface(req);
      auto message    = doc.dump(DEFAULT_INDENT);

      // the called function might have closed the connections
      if(incoming.generation != connectionGeneration)
      {
        DEBUG_OUTPUT("Discarding the reply to a closed connection");
        return;
      }

      outgoingQueue.push(
          {req->GetCallerIdentity(), message, incoming.generation});
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
}

void MessageHandler::DiscardQueuedMessages()
{
  connectionGeneration++;

  incomingQueue.apply_to_all([](const IncomingRequest &) {});
  outgoingQueue.apply_to_all([](const OutgoingReply &) {});
}

bool MessageHandler::IsRunning() const
{
  LockGuard lock(threadMutex);

  return m_thread.joinable();
}

void MessageHandler::HandleAllQueuedMessages()
{
  if(!RunningInMainThread() || incomingQueue.empty())
  {
    return;
  }

  auto msg = fmt::format("IDLE event messages: #{}", incomingQueue.size());
  GlobalData::Instance().AddLogEntry(msg);

  incomingQueue.apply_to_all(CallAndQueueReply);
}

MessageHandler::~MessageHandler()
{
  Stop();
}
