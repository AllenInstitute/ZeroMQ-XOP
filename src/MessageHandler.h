#pragma once

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class MessageHandler
{
public:
  MessageHandler(const MessageHandler &)            = delete;
  MessageHandler(MessageHandler &&)                 = delete;
  MessageHandler &operator=(const MessageHandler &) = delete;
  MessageHandler &operator=(MessageHandler &&)      = delete;

  /// Access to singleton-type global object
  static MessageHandler &Instance()
  {
    static MessageHandler obj;
    return obj;
  }

  void Start();
  void Stop();
  void HandleAllQueuedMessages();

  /// Discard the requests and replies of the current connections, called
  /// when the connections are closed
  void DiscardQueuedMessages();
  [[nodiscard]] bool IsRunning() const;

  /// @brief Throws HANDLER_RUNNING if the handler runs
  ///
  /// The handler can not be started while the returned lock is held.
  [[nodiscard]] std::unique_lock<std::recursive_mutex> LockStopped() const;

private:
  MessageHandler() = default;
  ~MessageHandler();

  class thread;
  std::thread m_thread;
};
