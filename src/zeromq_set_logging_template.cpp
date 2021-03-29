#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_set_logging_template(string jsonString)
extern "C" int zeromq_set_logging_template(zeromq_set_logging_templateParams *p)
{
  BEGIN_OUTER_CATCH

  const auto jsonString = GetStringFromHandle(p->jsonString);
  WMDisposeHandle(p->jsonString);

  DEBUG_OUTPUT("jsonString = {}", jsonString);
  GlobalData::Instance().SetLoggingTemplate(jsonString);

  END_OUTER_CATCH
}
