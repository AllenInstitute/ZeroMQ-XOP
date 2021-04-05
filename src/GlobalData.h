#pragma once

#include <mutex>
#include <memory>
#include "Logging.h"
#include "ConcurrentQueue.h"
#include "ConcurrentXOPNotice.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

enum class SocketTypes
{
  Client,
  Server
};

using AllSocketTypesArray = std::array<SocketTypes, 2>;

AllSocketTypesArray GetAllSocketTypes();

class GlobalData
{
public:
  /// Access to singleton-type global object
  static GlobalData &Instance()
  {
    static GlobalData globData;
    return globData;
  }

  void *ZMQSocket(SocketTypes st);
  bool HasBindsOrConnections(SocketTypes st);

  void SetDebugFlag(bool val);
  bool GetDebugFlag() const;

  void SetRecvBusyWaitingFlag(bool val);
  bool GetRecvBusyWaitingFlag() const;

  void CloseConnections();
  void AddToListOfBindsOrConnections(const std::string &localPoint,
                                     SocketTypes st);
  ConcurrentQueue<OutputMessagePtr> &GetXOPNoticeQueue();

  std::recursive_mutex &GetMutex(SocketTypes st);

  void SetLoggingFlag(bool val);
  bool GetLoggingFlag() const;

  void AddLogEntry(const json &doc, MessageDirection dir);
  void AddLogEntry(const std::string &str, MessageDirection dir);
  void SetLoggingTemplate(const std::string &loggingTemplate);
  void InitLogging();

private:
  GlobalData();
  ~GlobalData()                  = default;
  GlobalData(const GlobalData &) = delete;
  GlobalData &operator=(const GlobalData &) = delete;

  struct SocketTypeData
  {
    std::vector<std::string> m_list; // list of connections or binds
    void *m_zmq_socket{};
    std::recursive_mutex m_mutex;
  };

  bool HasSocket(SocketTypes st);

  SocketTypeData &GetSocketTypeData(SocketTypes st);

  SocketTypeData m_client, m_server, m_pub, m_sub;
  std::recursive_mutex m_settingsMutex;

  bool m_debugging;
  bool m_busyWaiting;
  bool m_logging;

  ConcurrentQueue<OutputMessagePtr> m_queue;
  std::unique_ptr<Logging> m_loggingSink;
  std::recursive_mutex m_loggingLock;
  void *zmq_context;
};

template <>
struct fmt::formatter<SocketTypes> : fmt::formatter<std::string>
{
  // parse is inherited from formatter<std::string>.
  template <typename FormatContext>
  auto format(SocketTypes st, FormatContext &ctx)
  {
    std::string name;
    switch(st)
    {
    case SocketTypes::Client:
      name = "Client";
      break;
    case SocketTypes::Server:
      name = "Server";
      break;
    }

    return formatter<std::string>::format(name, ctx);
  }
};
