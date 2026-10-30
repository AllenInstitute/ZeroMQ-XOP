#ifndef MESSAGE_HANDLER_PAUSE_GUARD_H
#define MESSAGE_HANDLER_PAUSE_GUARD_H

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

/// @brief Stops the running message handler for its lifetime
///
/// Must not be created while holding GetMutex(SocketTypes::Server).
class MessageHandlerPauseGuard
{
public:
  explicit MessageHandlerPauseGuard(bool enable);
  ~MessageHandlerPauseGuard();

  MessageHandlerPauseGuard(const MessageHandlerPauseGuard &) = delete;
  MessageHandlerPauseGuard &
  operator=(const MessageHandlerPauseGuard &)                      = delete;
  MessageHandlerPauseGuard(MessageHandlerPauseGuard &&)            = delete;
  MessageHandlerPauseGuard &operator=(MessageHandlerPauseGuard &&) = delete;

private:
  bool m_wasRunning;
};

#endif // MESSAGE_HANDLER_PAUSE_GUARD_H
