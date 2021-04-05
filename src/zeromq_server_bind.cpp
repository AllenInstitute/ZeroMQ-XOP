#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_server_bind(string localPoint)
extern "C" int zeromq_server_bind(zeromq_server_bindParams *p)
{
  BEGIN_OUTER_CATCH

  const auto localPoint = GetStringFromHandle(p->localPoint);
  WMDisposeHandle(p->localPoint);
  p->localPoint = nullptr;

  GET_SOCKET(socket, SocketTypes::Server);
  const auto rc = zmq_bind(socket.get(), localPoint.c_str());
  ZEROMQ_ASSERT(rc == 0);

  DEBUG_OUTPUT("localPoint={}, rc={}", localPoint, rc);
  GlobalData::Instance().AddToListOfBindsOrConnections(
      GetLastEndPoint(socket.get()), SocketTypes::Server);

  END_OUTER_CATCH
}
