#include "ZeroMQ.h"

#include "HeartbeatPublisher.h"
#include "MessageHandler.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_test_hb_stopped_by_stop()
//
// Regression test for zeromq_stop() forgetting to stop HeartbeatPublisher
// before GlobalData::CloseConnections() tears down the Publisher socket
extern "C" int
zeromq_test_hb_stopped_by_stop(zeromq_test_hb_stopped_by_stopParams *p)
{
  BEGIN_OUTER_CATCH

  // ensure a known, stopped baseline
  MessageHandler::Instance().Stop();
  HeartbeatPublisher::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  // HeartbeatPublisher only actually sends once a Publisher socket is bound
  {
    GET_SOCKET(socket, SocketTypes::Publisher);
    auto rc = zmq_bind(socket.get(), "tcp://127.0.0.1:*");
    ZEROMQ_ASSERT(rc == 0);
  }
  GlobalData::Instance().AddToListOfBindsOrConnections("tcp://127.0.0.1:*",
                                                       SocketTypes::Publisher);

  HeartbeatPublisher::Instance().Start();
  ASSERT(HeartbeatPublisher::Instance().IsRunning());

  zeromq_stopParams stopParams{};
  auto rc = zeromq_stop(&stopParams);
  ASSERT(rc == 0);

  ASSERT(!HeartbeatPublisher::Instance().IsRunning());

  HeartbeatPublisher::Instance().Start();

  END_OUTER_CATCH
}
