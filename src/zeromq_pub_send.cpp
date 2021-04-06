
#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_pub_send(string filter, string msg)
extern "C" int zeromq_pub_send(zeromq_pub_sendParams *p)
{
  BEGIN_OUTER_CATCH

  const auto msg = GetStringFromHandle(p->msg);
  WMDisposeHandle(p->msg);
  p->msg = nullptr;

  const auto filter = GetStringFromHandle(p->filter);
  WMDisposeHandle(p->filter);
  p->filter = nullptr;

  GlobalData::Instance().AddLogEntry(filter + ":" + msg,
                                     MessageDirection::Outgoing);
  ZeroMQPublisherSend(filter, msg);

  END_OUTER_CATCH
}
