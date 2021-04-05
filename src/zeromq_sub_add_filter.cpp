#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_sub_add_filter(string filter)
extern "C" int zeromq_sub_add_filter(zeromq_sub_add_filterParams *p)
{
  BEGIN_OUTER_CATCH
  const auto filter = GetStringFromHandle(p->filter);
  WMDisposeHandle(p->filter);
  p->filter = nullptr;

  GlobalData::Instance().AddSubscriberMessageFilter(filter);

  END_OUTER_CATCH
}
