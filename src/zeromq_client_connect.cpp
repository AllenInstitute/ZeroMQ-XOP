#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_client_connect(string remotePoint)
extern "C" int zeromq_client_connect(zeromq_client_connectParams *p)
{
  BEGIN_OUTER_CATCH

  const auto remotePoint = GetStringFromHandle(p->remotePoint);
  WMDisposeHandle(p->remotePoint);
  p->remotePoint = nullptr;

  GET_SOCKET(socket, SocketTypes::Client);
  const auto rc = zmq_connect(socket.get(), remotePoint.c_str());
  ZEROMQ_ASSERT(rc == 0);

  DEBUG_OUTPUT("remotePoint={}, rc={}", remotePoint, rc);
  GlobalData::Instance().AddToListOfBindsOrConnections(
      GetLastEndPoint(socket.get()), SocketTypes::Client);

  END_OUTER_CATCH
}
