#pragma once

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

/// @param A object name to create
#define GET_CLIENT_SOCKET(A)                                                   \
  SocketWithMutex A(GlobalData::Instance().ZMQClientSocket(),                  \
                    GlobalData::Instance().m_clientMutex);

// DebugOutput(fmt::format("{}: Trying to lock client socket\n", __func__));

/// @param A object name to create
#define GET_SERVER_SOCKET(A)                                                   \
  SocketWithMutex A(GlobalData::Instance().ZMQServerSocket(),                  \
                    GlobalData::Instance().m_serverMutex);

// DebugOutput(fmt::format("{}: Trying to lock server socket\n", __func__));

class SocketWithMutex
{
public:
  SocketWithMutex(void *s, std::recursive_mutex &mutex)
      : m_lock(mutex), m_plainSocket(s)
  {
    // DebugOutput(fmt::format("{}: Locking {}\n", __func__, m_plainSocket));
  }

  ~SocketWithMutex()
  {
    // DebugOutput(fmt::format("{}: Unlocking {}\n", __func__, m_plainSocket));
  }

  SocketWithMutex(const SocketWithMutex &) = delete;
  SocketWithMutex &operator=(const SocketWithMutex &) = delete;

  void *get()
  {
    return m_plainSocket;
  }

private:
  LockGuard m_lock;
  void *m_plainSocket;
};
