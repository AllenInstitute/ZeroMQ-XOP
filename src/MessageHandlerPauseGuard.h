#pragma once

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

/// @brief RAII guard that temporarily stops the message handler (if
///        running) for its lifetime, restarting it on destruction
///
/// The message handler's worker thread continuously polls the Server
/// socket (roughly once per ZMQ_RCVTIMEO, currently 1ms); an operation
/// that reconfigures that same socket (zmq_bind, zmq_setsockopt) is
/// dispatched to libzmq's internal I/O thread and can be starved for that
/// thread's attention by the concurrent polling -- observed live anywhere
/// from single-digit milliseconds up to multiple minutes. Wrap any
/// Server-socket-reconfiguring operation in this guard.
///
/// Only construct this while GetMutex(SocketTypes::Server) is *not* held --
/// MessageHandler::Stop()/Start() don't need it themselves, but a caller
/// already holding it (e.g. from within GET_SOCKET(..., SocketTypes::Server))
/// would deadlock against the worker thread's own attempt to acquire it.
///
/// Pass enable = false to make this a no-op (e.g. when the caller knows the
/// operation at hand doesn't touch the Server socket).
class MessageHandlerPauseGuard
{
public:
  explicit MessageHandlerPauseGuard(bool enable);
  ~MessageHandlerPauseGuard();

  MessageHandlerPauseGuard(const MessageHandlerPauseGuard &) = delete;
  MessageHandlerPauseGuard &
  operator=(const MessageHandlerPauseGuard &) = delete;

private:
  bool m_wasRunning;
};
