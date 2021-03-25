#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// variable zeromq_set(variable flags)
extern "C" int zeromq_set(zeromq_setParams *p)
{
  BEGIN_OUTER_CATCH

  DEBUG_OUTPUT("flags={}", p->flags);
  ApplyFlags(p->flags);

  END_OUTER_CATCH
}
