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

/// Waits until the message arrives, the receive itself waits up to 1 ms
template <typename T>
std::string WaitForMessage(T receive)
{
  zmq_msg_t msg;
  auto rc = zmq_msg_init(&msg);
  ZEROMQ_ASSERT(rc == 0);

  const auto start = std::chrono::steady_clock::now();

  for(;;)
  {
    if(receive(&msg) >= 0)
    {
      break;
    }

    ZEROMQ_ASSERT(zmq_errno() == EAGAIN);
    ASSERT(std::chrono::steady_clock::now() - start < TIMEOUT_PER_CALL);
  }

  auto str = CreateStringFromZMsg(&msg);
  zmq_msg_close(&msg);

  return str;
}

} // anonymous namespace

// variable zeromq_test_send_contention(variable numCalls)
extern "C" int zeromq_test_send_contention(zeromq_test_send_contentionParams *p)
{
  BEGIN_OUTER_CATCH

  const auto numCalls = To<int>(p->numCalls);
  ASSERT(numCalls >= 1);

  // ensure a known, stopped baseline
  MessageHandler::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  std::string endpoint;
  {
    GET_SOCKET(socket, SocketTypes::Server);
    auto rc = zmq_bind(socket.get(), "tcp://127.0.0.1:*");
    ZEROMQ_ASSERT(rc == 0);
    endpoint = GetLastEndPoint(socket.get());
  }
  GlobalData::Instance().AddToListOfBindsOrConnections(endpoint,
                                                       SocketTypes::Server);

  {
    GET_SOCKET(clientSocket, SocketTypes::Client);
    auto rc = zmq_connect(clientSocket.get(), endpoint.c_str());
    ZEROMQ_ASSERT(rc == 0);
  }
  GlobalData::Instance().AddToListOfBindsOrConnections(endpoint,
                                                       SocketTypes::Client);

  // the server learns the identity of the client with its first message
  auto rc = ZeroMQClientSend("hello");
  ZEROMQ_ASSERT(rc >= 0);

  zmq_msg_t identityMsg;
  rc = zmq_msg_init(&identityMsg);
  ZEROMQ_ASSERT(rc == 0);

  const auto hello =
      WaitForMessage([&identityMsg](zmq_msg_t *msg)
                     { return ZeroMQServerReceive(&identityMsg, msg); });
  ASSERT(hello == "hello");

  const auto identity = CreateStringFromZMsg(&identityMsg);
  zmq_msg_close(&identityMsg);

  MessageHandler::Instance().Start();
  ASSERT(MessageHandler::Instance().IsRunning());

  int numStalled = 0;

  for(int i = 0; i < numCalls; i++)
  {
    const auto payload = std::to_string(i);

    // a separate thread, so a stalled call does not block Igor Pro
    auto sent = std::async(std::launch::async, [&identity, &payload]
                           { return ZeroMQServerSend(identity, payload); });

    if(sent.wait_for(TIMEOUT_PER_CALL) != std::future_status::ready)
    {
      numStalled++;

      // releases the mutex the handler holds, so the call returns
      MessageHandler::Instance().Stop();
      break;
    }

    rc = sent.get();
    ZEROMQ_ASSERT(rc >= 0);

    const auto received = WaitForMessage(ZeroMQClientReceive);
    ASSERT(received == payload);
  }

  // restore a clean, stopped and unbound baseline
  MessageHandler::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  p->result = numStalled;

  END_OUTER_CATCH
}
