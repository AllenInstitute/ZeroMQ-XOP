#pragma once

#include <mutex>
#include <memory>

#include "ConcurrentQueue.h"
#include "ConcurrentXOPNotice.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class GlobalData
{
public:
  /// Access to singleton-type global object
  static GlobalData &Instance()
  {
    static GlobalData globData;
    return globData;
  }

  void *ZMQClientSocket();
  bool HasClientSocket();

  void *ZMQServerSocket();
  bool HasServerSocket();

  void SetDebugFlag(bool val);
  bool GetDebugFlag() const;

  void SetRecvBusyWaitingFlag(bool val);
  bool GetRecvBusyWaitingFlag() const;

  void CloseConnections();
  bool HasBinds();
  void AddToListOfBinds(const std::string &localPoint);
  bool HasConnections();
  void AddToListOfConnections(const std::string &remotePoint);
  ConcurrentQueue<OutputMessagePtr> &GetXOPNoticeQueue();

  std::recursive_mutex m_clientMutex, m_serverMutex;

private:
  GlobalData();
  ~GlobalData()                  = default;
  GlobalData(const GlobalData &) = delete;
  GlobalData &operator=(const GlobalData &) = delete;

  void *zmq_context;
  void *zmq_client_socket{};
  void *zmq_server_socket{};
  std::vector<std::string> m_binds, m_connections;
  std::recursive_mutex m_settingsMutex;

  bool m_debugging;
  bool m_busyWaiting;

  ConcurrentQueue<OutputMessagePtr> m_queue;
};
