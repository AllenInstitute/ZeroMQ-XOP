#include "ZeroMQ.h"
#include "HeartbeatPublisher.h"
#include "RequestInterface.h"

#include <chrono>
#include <condition_variable>
#include <thread>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

using namespace std::chrono_literals;
std::recursive_mutex threadMutex;
bool threadShouldFinish;
std::mutex threadShouldFinishMutex;
std::condition_variable threadShouldFinishCondition;

/// Wait until the next heartbeat is due
///
/// @return true if the thread should finish
bool WaitForNextHeartbeat()
{
  std::unique_lock<std::mutex> lock(threadShouldFinishMutex);

  return threadShouldFinishCondition.wait_for(lock, 5s, []
                                              { return threadShouldFinish; });
}

void WorkerThread()
{
  DEBUG_OUTPUT("Begin");

  for(;;)
  {
    try
    {
      // check if stop is requested
      {
        const std::scoped_lock<std::mutex> lock(threadShouldFinishMutex);
        if(threadShouldFinish)
        {
          DEBUG_OUTPUT("Exiting");
          break;
        }
      }

      if(GlobalData::Instance().HasBindsOrConnections(SocketTypes::Publisher))
      {
        SendStorageVec sendStorage;
        sendStorage.emplace_back(SendStorage{"heartbeat"});
        sendStorage.emplace_back(SendStorage{""});

        auto rc = ZeroMQPublisherSend(sendStorage);

        if(rc)
        {
          EMERGENCY_OUTPUT(
              "Error sending heartbeat publisher message with rc = {}", rc);
        }
      }
    }
    catch(const IgorException &e)
    {
      EMERGENCY_OUTPUT(
          "Caught IgorException with what = \"{}\". This must NOT happen!",
          e.what());
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

    if(WaitForNextHeartbeat())
    {
      DEBUG_OUTPUT("Exiting");
      break;
    }
  }
}

} // anonymous namespace

void HeartbeatPublisher::Start()
{
  const LockGuard lock(threadMutex);

  if(m_thread.joinable())
  {
    throw IgorException(INTERNAL_ERROR,
                        "Can not start heartbeat publisher twice");
  }

  DEBUG_OUTPUT("Trying to start.");

  {
    const std::scoped_lock<std::mutex> innerLock(threadShouldFinishMutex);
    threadShouldFinish = false;
  }

  auto t = std::thread(WorkerThread);
  m_thread.swap(t);
}

void HeartbeatPublisher::Stop()
{
  const LockGuard lock(threadMutex);

  if(!m_thread.joinable())
  {
    return;
  }

  DEBUG_OUTPUT("Shutting down.");

  {
    const std::scoped_lock<std::mutex> innerLock(threadShouldFinishMutex);
    threadShouldFinish = true;
  }

  threadShouldFinishCondition.notify_all();

  m_thread.join();
}

bool HeartbeatPublisher::IsRunning() const
{
  const LockGuard lock(threadMutex);

  return m_thread.joinable();
}

HeartbeatPublisher::~HeartbeatPublisher()
{
  Stop();
}
