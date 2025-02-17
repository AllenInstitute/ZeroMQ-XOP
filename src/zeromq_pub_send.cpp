
#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_pub_send(string filter, string msg)
extern "C" int zeromq_pub_send(zeromq_pub_sendParams *p)
{
  BEGIN_OUTER_CATCH

  const auto msg = GetStringFromHandleWithDispose(p->msg);
  p->msg         = nullptr;

  const auto filter = GetStringFromHandleWithDispose(p->filter);
  p->filter         = nullptr;

  GlobalData::Instance().AddLogEntry(filter + ":" + msg,
                                     MessageDirection::Outgoing);

  SendStorageVec sendStorage;
  sendStorage.emplace_back(SendStorage{filter});
  sendStorage.emplace_back(SendStorage{msg});

  ZeroMQPublisherSend(sendStorage);

  END_OUTER_CATCH
}
