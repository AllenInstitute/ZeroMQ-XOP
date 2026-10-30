#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

SocketTypes ConvertSocketType(double socketType)
{
  const auto value = lockToIntegerRange<int>(socketType);

  switch(value)
  {
  case ZeroMQ_SOCKET_TYPE::CLIENT:
    return SocketTypes::Client;
  case ZeroMQ_SOCKET_TYPE::SERVER:
    return SocketTypes::Server;
  case ZeroMQ_SOCKET_TYPE::PUBLISHER:
    return SocketTypes::Publisher;
  case ZeroMQ_SOCKET_TYPE::SUBSCRIBER:
    return SocketTypes::Subscriber;
  default:
    throw IgorException(INVALID_ARG,
                        fmt::format("Unknown socket type {}.", value));
  }
}

} // anonymous namespace

// WAVE zeromq_get_bound_endpoints(variable socketType)
extern "C" int zeromq_get_bound_endpoints(zeromq_get_bound_endpointsParams *p)
{
  BEGIN_OUTER_CATCH

  const auto st        = ConvertSocketType(p->socketType);
  const auto endpoints = GlobalData::Instance().GetListOfBindsOrConnections(st);

  DEBUG_OUTPUT("type={}, endpoints={}", st, endpoints.size());

  auto wv = MakeFreeWave({To<CountInt>(endpoints.size())}, TEXT_WAVE_TYPE);

  std::vector<IndexInt> dims(MAX_DIMENSIONS, 0);
  for(const auto &endpoint : endpoints)
  {
    SetWaveElement(wv, dims, endpoint);
    dims[0]++;
  }

  p->result = wv;

  END_OUTER_CATCH
}
