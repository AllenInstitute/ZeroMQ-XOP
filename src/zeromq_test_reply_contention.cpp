#include "ZeroMQ.h"

#include "MessageHandler.h"

#include <chrono>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

using namespace std::chrono_literals;

constexpr auto TIMEOUT_PER_REQUEST = 5s;

} // anonymous namespace

// variable zeromq_test_reply_contention(variable numRequests)
extern "C" int
zeromq_test_reply_contention(zeromq_test_reply_contentionParams *p)
{
  BEGIN_OUTER_CATCH

  const auto numRequests = To<int>(p->numRequests);
  ASSERT(numRequests >= 1);

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

  // a Client socket plays the role of an external caller (e.g. MIES/ZBR)
  {
    GET_SOCKET(clientSocket, SocketTypes::Client);
    auto rc = zmq_connect(clientSocket.get(), endpoint.c_str());
    ZEROMQ_ASSERT(rc == 0);
  }
  GlobalData::Instance().AddToListOfBindsOrConnections(endpoint,
                                                       SocketTypes::Client);

  json req;
  req["version"]              = 1;
  req["CallFunction"]["name"] = "ZeroMQTestNonExistentFunctionXYZ";
  const auto payload          = req.dump();

  for(int i = 0; i < numRequests; i++)
  {
    auto sendRc = ZeroMQClientSend(payload);
    ZEROMQ_ASSERT(sendRc >= 0);

    zmq_msg_t reply;
    auto rc = zmq_msg_init(&reply);
    ZEROMQ_ASSERT(rc == 0);

    const auto start = std::chrono::steady_clock::now();

    for(;;)
    {
      // same as the IDLE event
      MessageHandler::Instance().HandleAllQueuedMessages();

      auto numBytes = ZeroMQClientReceive(&reply);

      if(numBytes >= 0)
      {
        break;
      }

      ZEROMQ_ASSERT(zmq_errno() == EAGAIN);

      if(std::chrono::steady_clock::now() - start > TIMEOUT_PER_REQUEST)
      {
        zmq_msg_close(&reply);
        throw IgorException(MSGHANDLER_REPLY_STALL);
      }
    }

    zmq_msg_close(&reply);
  }

  // restore a clean, stopped and unbound baseline
  MessageHandler::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  END_OUTER_CATCH
}
