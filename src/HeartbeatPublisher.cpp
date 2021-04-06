#include "ZeroMQ.h"
#include "HeartbeatPublisher.h"
#include "RequestInterface.h"

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

void WorkerThread()
{
  DEBUG_OUTPUT("Begin");

  {
    // initialize to false
    LockGuard lock(threadShouldFinishMutex);
    threadShouldFinish = false;
  }

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

      if(GlobalData::Instance().HasBindsOrConnections(SocketTypes::Publisher))
      {
        auto rc = ZeroMQPublisherSend("heartbeat", "");

        if(rc)
        {
          EMERGENCY_OUTPUT(
              "Error sending heartbeat publisher message with rc = {}", rc);
          continue;
        }
      }

      std::this_thread::sleep_for(5s);
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
  }
}

} // anonymous namespace

void HeartbeatPublisher::Start()
{
  LockGuard lock(threadMutex);

  if(m_thread.joinable())
  {
    throw IgorException(INTERNAL_ERROR,
                        "Can not start heartbeat publisher twice");
  }

  DEBUG_OUTPUT("Trying to start.");

  auto t = std::thread(WorkerThread);
  m_thread.swap(t);
}

void HeartbeatPublisher::Stop()
{
  LockGuard lock(threadMutex);

  if(!m_thread.joinable())
  {
    return;
  }

  DEBUG_OUTPUT("Shutting down.");

  {
    LockGuard innerLock(threadShouldFinishMutex);
    threadShouldFinish = true;
  }

  m_thread.join();
}

HeartbeatPublisher::~HeartbeatPublisher()
{
  Stop();
}
