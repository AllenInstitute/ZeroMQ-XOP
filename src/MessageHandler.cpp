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

/// A request, or the error reply to a message which is not a valid request
///
/// The error replies also go through the queue to keep the order of the
/// replies.
struct IncomingRequest
{
  RequestInterfacePtr request;
  std::string identity;
  std::string errorReply;
  uint64_t generation;
};

ConcurrentQueue<IncomingRequest> &GetIncomingQueue()
{
  static ConcurrentQueue<IncomingRequest> queue;
  return queue;
}

struct OutgoingReply
{
  std::string identity;
  std::string message;
  uint64_t generation;
};

ConcurrentQueue<OutgoingReply> &GetOutgoingQueue()
{
  static ConcurrentQueue<OutgoingReply> queue;
  return queue;
}

void QueueErrorReply(const std::string &identity,
                     const IgorException &exception)
{
  const json reply = exception;

  GetIncomingQueue().push(
      {nullptr, identity, reply.dump(DEFAULT_INDENT), connectionGeneration});
}

void DrainOutgoingReplies()
{
  if(RunningInMainThread() || GetOutgoingQueue().empty())
  {
    return;
  }

  GetOutgoingQueue().apply_to_all(
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
        const LockGuard lock(threadShouldFinishMutex);
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
        QueueErrorReply(CreateStringFromZMsg(&identityMsg), e);
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
          GetIncomingQueue().push({req, identity, "", connectionGeneration});
        }
        catch(const std::bad_alloc &)
        {
          throw RequestInterfaceException(REQ_OUT_OF_MEMORY);
        }
      }
      catch(const IgorException &e)
      {
        QueueErrorReply(identity, e);
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

// FMT_STRING checks the format string at compile time, the format_error
// thrown when checking it at runtime can not happen
// NOLINTNEXTLINE(bugprone-exception-escape)
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

      if(!incoming.request)
      {
        GetOutgoingQueue().push(
            {incoming.identity, incoming.errorReply, incoming.generation});
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

      GetOutgoingQueue().push(
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
  const LockGuard lock(threadMutex);

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
    const LockGuard innerLock(threadShouldFinishMutex);
    threadShouldFinish = false;
  }

  DEBUG_OUTPUT("Before WorkerThread() start.");

  auto worker = std::thread(WorkerThread);
  m_thread.swap(worker);
}

void MessageHandler::Stop()
{
  const LockGuard lock(threadMutex);

  if(!m_thread.joinable())
  {
    return;
  }

  DEBUG_OUTPUT("Shutting down the handler.");

  {
    const LockGuard innerLock(threadShouldFinishMutex);
    threadShouldFinish = true;
  }

  m_thread.join();
}

void MessageHandler::DiscardQueuedMessages()
{
  connectionGeneration++;

  GetIncomingQueue().apply_to_all([](const IncomingRequest &) {});
  GetOutgoingQueue().apply_to_all([](const OutgoingReply &) {});
}

bool MessageHandler::IsRunning() const
{
  const LockGuard lock(threadMutex);

  return m_thread.joinable();
}

void MessageHandler::HandleAllQueuedMessages()
{
  if(!RunningInMainThread() || GetIncomingQueue().empty())
  {
    return;
  }

  auto msg = fmt::format("IDLE event messages: #{}", GetIncomingQueue().size());
  GlobalData::Instance().AddLogEntry(msg);

  GetIncomingQueue().apply_to_all(CallAndQueueReply);
}

// FMT_STRING checks the format string at compile time, the format_error
// thrown when checking it at runtime can not happen
// NOLINTNEXTLINE(bugprone-exception-escape)
MessageHandler::~MessageHandler()
{
  try
  {
    Stop();
  }
  catch(const std::exception &e)
  {
    EMERGENCY_OUTPUT("Could not stop the thread: {}", e.what());
  }
}
