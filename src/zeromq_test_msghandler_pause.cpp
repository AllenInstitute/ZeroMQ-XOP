#include "ZeroMQ.h"

#include "MessageHandler.h"
#include "MessageHandlerPauseGuard.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_test_msghandler_pause()
//
// Regression test for MessageHandlerPauseGuard: establishes a dummy Server
// bind, starts the message handler, and checks that constructing the guard
// stops it, that destroying the guard restarts it,
// and that a disabled guard is a true no-op.
extern "C" int
zeromq_test_msghandler_pause(zeromq_test_msghandler_pauseParams *p)
{
  BEGIN_OUTER_CATCH

  // ensure a known, stopped baseline
  MessageHandler::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  // a Server bind is required before the handler is allowed to start
  {
    GET_SOCKET(socket, SocketTypes::Server);
    auto rc = zmq_bind(socket.get(), "tcp://127.0.0.1:*");
    ZEROMQ_ASSERT(rc == 0);
  }
  GlobalData::Instance().AddToListOfBindsOrConnections("tcp://127.0.0.1:*",
                                                       SocketTypes::Server);

  MessageHandler::Instance().Start();
  ASSERT(MessageHandler::Instance().IsRunning());

  {
    MessageHandlerPauseGuard pauseGuard(true);
    ASSERT(!MessageHandler::Instance().IsRunning());
  }
  ASSERT(MessageHandler::Instance().IsRunning());

  {
    MessageHandlerPauseGuard pauseGuard(false);
    ASSERT(MessageHandler::Instance().IsRunning());
  }
  ASSERT(MessageHandler::Instance().IsRunning());

  // restore a clean, stopped and unbound baseline
  MessageHandler::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  END_OUTER_CATCH
}
