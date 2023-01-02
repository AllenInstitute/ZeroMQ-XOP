#pragma once

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

#define GET_SOCKET(A, ST)                                                      \
  SocketWithMutex A(GlobalData::Instance().ZMQSocket(ST),                      \
                    GlobalData::Instance().GetMutex(ST));

class SocketWithMutex
{
public:
  SocketWithMutex(void *s, std::recursive_mutex &mutex)
      : m_lock(mutex), m_plainSocket(s)
  {
    // DEBUG_OUTPUT("Locking {}",  m_plainSocket);
  }

  ~SocketWithMutex()
  {
    // DEBUG_OUTPUT("Unlocking {}",  m_plainSocket);
  }

  SocketWithMutex(const SocketWithMutex &)            = delete;
  SocketWithMutex &operator=(const SocketWithMutex &) = delete;

  void *get()
  {
    return m_plainSocket;
  }

private:
  LockGuard m_lock;
  void *m_plainSocket;
};
