#pragma once

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

#define GET_SOCKET(A, ST) SocketWithMutex A(ST);

/// @brief RAII holder for a ZMQ socket, locked for its entire lifetime
///
/// Acquiring GetMutex(st) and fetching/creating the socket happen as a
/// single critical section (member initialization order guarantees m_lock
/// is constructed, i.e. the mutex is held, before m_plainSocket's
/// initializer runs) -- unlike calling GlobalData::GetOrCreateSocket(st) and
/// only locking afterward, which would leave a window for another thread to
/// close the socket in between and hand this constructor a stale pointer.
class SocketWithMutex
{
public:
  explicit SocketWithMutex(SocketTypes st)
      : m_lock(GlobalData::Instance().GetMutex(st)),
        m_plainSocket(GlobalData::Instance().GetOrCreateSocket(st))
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
  // Declaration order matters: m_lock must be constructed (i.e. the mutex
  // locked) before m_plainSocket's initializer runs.
  LockGuard m_lock;
  void *m_plainSocket;
};
