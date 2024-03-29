#include "ZeroMQ.h"
#include "MessageHandler.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_handler_stop()
extern "C" int zeromq_handler_stop(zeromq_handler_stopParams *p)
{
  BEGIN_OUTER_CATCH

  MessageHandler::Instance().Stop();

  END_OUTER_CATCH
}
