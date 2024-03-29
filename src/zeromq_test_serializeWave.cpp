#include "ZeroMQ.h"
#include "CallFunctionOperation.h"
#include "RequestInterface.h"
#include "SerializeWave.h"
#include <string>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// string zeromq_test_serializeWave(WAVE wv)
extern "C" int zeromq_test_serializeWave(zeromq_test_serializeWaveParams *p)
{
  BEGIN_OUTER_CATCH

  const auto str = SerializeWave(p->wv).dump(DEFAULT_INDENT);

  DEBUG_OUTPUT("output={:.255s}", str);

  auto len  = str.size();
  p->result = WMNewHandle(len);
  ASSERT(p->result != nullptr);
  memcpy(*(p->result), str.c_str(), len);

  END_OUTER_CATCH
}
