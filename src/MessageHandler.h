#pragma once

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class MessageHandler
{
public:
  /// Access to singleton-type global object
  static MessageHandler &Instance()
  {
    static MessageHandler obj;
    return obj;
  }

  void Start();
  void Stop();
  void HandleAllQueuedMessages();

private:
  MessageHandler() = default;
  ~MessageHandler();
  MessageHandler(const MessageHandler &) = delete;
  MessageHandler &operator=(const MessageHandler &) = delete;

  class thread;
  std::thread m_thread;
};
