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

} // anonymous namespace

// variable zeromq_test_endpoint_contention(variable numCalls)
extern "C" int
zeromq_test_endpoint_contention(zeromq_test_endpoint_contentionParams *p)
{
  BEGIN_OUTER_CATCH

  const auto numCalls = To<int>(p->numCalls);
  ASSERT(numCalls >= 1);

  // ensure a known, stopped baseline
  MessageHandler::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  // a Server bind is required before the handler is allowed to start
  std::string endpoint;
  {
    GET_SOCKET(socket, SocketTypes::Server);
    auto rc = zmq_bind(socket.get(), "tcp://127.0.0.1:*");
    ZEROMQ_ASSERT(rc == 0);
    endpoint = GetLastEndPoint(socket.get());
  }
  GlobalData::Instance().AddToListOfBindsOrConnections(endpoint,
                                                       SocketTypes::Server);

  MessageHandler::Instance().Start();
  ASSERT(MessageHandler::Instance().IsRunning());

  int numStalled = 0;

  for(int i = 0; i < numCalls; i++)
  {
    // a separate thread, so a stalled call does not block Igor Pro
    auto endpoints =
        std::async(std::launch::async,
                   []
                   {
                     return GlobalData::Instance().GetListOfBindsOrConnections(
                         SocketTypes::Server);
                   });

    if(endpoints.wait_for(TIMEOUT_PER_CALL) != std::future_status::ready)
    {
      numStalled++;

      // releases the mutexes the handler holds, so the call returns
      MessageHandler::Instance().Stop();
      break;
    }

    const auto list = endpoints.get();
    ASSERT(list.size() == 1 && list[0] == endpoint);
  }

  // restore a clean, stopped and unbound baseline
  MessageHandler::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  p->result = numStalled;

  END_OUTER_CATCH
}
