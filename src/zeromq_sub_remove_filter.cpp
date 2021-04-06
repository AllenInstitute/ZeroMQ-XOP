#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_sub_remove_filter(string filter)
extern "C" int zeromq_sub_remove_filter(zeromq_sub_remove_filterParams *p)
{
  BEGIN_OUTER_CATCH

  const auto filter = GetStringFromHandle(p->filter);
  WMDisposeHandle(p->filter);
  p->filter = nullptr;

  GlobalData::Instance().RemoveSubscriberMessageFilter(filter);

  END_OUTER_CATCH
}
