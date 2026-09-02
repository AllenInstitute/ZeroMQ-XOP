#include "ZeroMQ.h"
#include "MessageHandler.h"
#include "HeartbeatPublisher.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_stop()
extern "C" int zeromq_stop(zeromq_stopParams *p)
{
  BEGIN_OUTER_CATCH

  // Both worker threads must be stopped before CloseConnections() tears down
  // the sockets they use -- HeartbeatPublisher polls/sends on the Publisher
  // socket independently of MessageHandler, and leaving it running here lets
  // its concurrent send collide with CloseConnections()'s zmq_unbind() on
  // that same socket, starving libzmq's I/O thread anywhere from
  // single-digit ms to multiple minutes (the same class of issue
  // MessageHandlerPauseGuard documents/fixes for zmq_bind/zmq_setsockopt).
  // HeartbeatPublisher is restarted by DoBindOrConnect() once the Publisher
  // socket is bound again.
  MessageHandler::Instance().Stop();
  HeartbeatPublisher::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  END_OUTER_CATCH
}
