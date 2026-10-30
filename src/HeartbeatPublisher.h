#pragma once

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class HeartbeatPublisher
{
public:
  HeartbeatPublisher(const HeartbeatPublisher &)            = delete;
  HeartbeatPublisher(HeartbeatPublisher &&)                 = delete;
  HeartbeatPublisher &operator=(const HeartbeatPublisher &) = delete;
  HeartbeatPublisher &operator=(HeartbeatPublisher &&)      = delete;

  /// Access to singleton-type global object
  static HeartbeatPublisher &Instance()
  {
    static HeartbeatPublisher obj;
    return obj;
  }

  void Start();
  void Stop();
  [[nodiscard]] bool IsRunning() const;

private:
  HeartbeatPublisher() = default;
  ~HeartbeatPublisher();

  class thread;
  std::thread m_thread;
};
