#include "ZeroMQ.h"

#include "MessageHandler.h"

#include <chrono>
#include <future>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

using namespace std::chrono_literals;

constexpr auto TIMEOUT_PER_CALL = 5s;

void BindServer()
{
  auto *handle = GetHandleFromString("tcp://127.0.0.1:*");
  ASSERT(handle != nullptr);

  DoBindOrConnect(handle, SocketTypes::Server);
}

} // anonymous namespace

// variable zeromq_test_bind_contention(variable numCalls)
extern "C" int zeromq_test_bind_contention(zeromq_test_bind_contentionParams *p)
{
  BEGIN_OUTER_CATCH

  const auto numCalls = To<int>(p->numCalls);
  ASSERT(numCalls >= 1);

  // ensure a known, stopped baseline
  MessageHandler::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  // a Server bind is required before the handler is allowed to start
  BindServer();

  MessageHandler::Instance().Start();
  ASSERT(MessageHandler::Instance().IsRunning());

  int numStalled = 0;

  for(int i = 0; i < numCalls; i++)
  {
    // a separate thread, so a stalled call does not block Igor Pro
    auto done = std::async(std::launch::async,
                           [i]
                           {
                             BindServer();
                             ToggleIPV6Support(i % 2 == 0);
                           });

    if(done.wait_for(TIMEOUT_PER_CALL) != std::future_status::ready)
    {
      numStalled++;

      // releases the mutex the handler holds, so the call returns
      MessageHandler::Instance().Stop();
      break;
    }

    done.get(); // rethrows

    ASSERT(MessageHandler::Instance().IsRunning());
  }

  if(numStalled == 0)
  {
    const auto endpoints =
        GlobalData::Instance().GetListOfBindsOrConnections(SocketTypes::Server);
    ASSERT(endpoints.size() == To<size_t>(numCalls + 1));
  }

  // restore a clean, stopped and unbound baseline
  MessageHandler::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  p->result = numStalled;

  END_OUTER_CATCH
}
