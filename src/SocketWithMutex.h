#ifndef SOCKET_WITH_MUTEX_H
#define SOCKET_WITH_MUTEX_H

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

#define GET_SOCKET(A, ST) SocketWithMutex A(ST);

/// @brief Holds the socket and its locked mutex
class SocketWithMutex
{
public:
  SocketWithMutex(const SocketWithMutex &)            = delete;
  SocketWithMutex(SocketWithMutex &&)                 = delete;
  SocketWithMutex &operator=(const SocketWithMutex &) = delete;
  SocketWithMutex &operator=(SocketWithMutex &&)      = delete;

  explicit SocketWithMutex(SocketTypes st)
      : m_lock(GlobalData::Instance().GetMutex(st)),
        m_plainSocket(GlobalData::Instance().GetOrCreateSocket(st))
  {
    // DEBUG_OUTPUT("Locking {}",  m_plainSocket);
  }

  ~SocketWithMutex() = default;

  void *get()
  {
    return m_plainSocket;
  }

private:
  // m_lock must be initialized before m_plainSocket
  LockGuard m_lock;
  void *m_plainSocket;
};

#endif // SOCKET_WITH_MUTEX_H
