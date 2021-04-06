#include "ZeroMQ.h"
#include "MessageHandler.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_stop()
extern "C" int zeromq_stop(zeromq_stopParams *p)
{
  BEGIN_OUTER_CATCH

  MessageHandler::Instance().Stop();
  GlobalData::Instance().CloseConnections();

  END_OUTER_CATCH
}
