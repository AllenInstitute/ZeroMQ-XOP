#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

SocketTypes ConvertSocketType(double socketType)
{
  using ut = std::underlying_type_t<SocketTypes>;

  const auto number = lockToIntegerRange<ut>(socketType);
  const auto value  = static_cast<SocketTypes>(number);

  switch(value)
  {
  case SocketTypes::Client:
  case SocketTypes::Server:
  case SocketTypes::Publisher:
  case SocketTypes::Subscriber:
    return value;
  }

  throw IgorException(INVALID_ARG,
                      fmt::format("Unknown socket type {}.", number));
}

} // anonymous namespace

// WAVE zeromq_get_bound_endpoints(variable socketType)
extern "C" int zeromq_get_bound_endpoints(zeromq_get_bound_endpointsParams *p)
{
  BEGIN_OUTER_CATCH

  const auto st        = ConvertSocketType(p->socketType);
  const auto endpoints = GlobalData::Instance().GetListOfBindsOrConnections(st);

  DEBUG_OUTPUT("type={}, endpoints={}", st, endpoints.size());

  if(endpoints.empty())
  {
    return 0;
  }

  auto *wv = MakeFreeWave({To<CountInt>(endpoints.size())}, TEXT_WAVE_TYPE);

  std::vector<IndexInt> dims(MAX_DIMENSIONS, 0);
  for(const auto &endpoint : endpoints)
  {
    SetWaveElement(wv, dims, endpoint);
    dims[ROWS]++;
  }

  p->result = wv;

  END_OUTER_CATCH
}
