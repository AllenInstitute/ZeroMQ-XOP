#pragma once

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class HeartbeatPublisher
{
public:
  /// Access to singleton-type global object
  static HeartbeatPublisher &Instance()
  {
    static HeartbeatPublisher obj;
    return obj;
  }

  void Start();
  void Stop();
  bool IsRunning() const;

private:
  HeartbeatPublisher() = default;
  ~HeartbeatPublisher();
  HeartbeatPublisher(const HeartbeatPublisher &)            = delete;
  HeartbeatPublisher &operator=(const HeartbeatPublisher &) = delete;

  class thread;
  std::thread m_thread;
};
