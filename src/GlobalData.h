#pragma once

#include "ConcurrentQueue.h"
#include "ConcurrentXOPNotice.h"
#include "Logging.h"
#include <atomic>
#include <memory>
#include <mutex>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

enum class SocketTypes
{
  Client,
  Server,
  Publisher,
  Subscriber
};

using AllSocketTypesArray = std::array<SocketTypes, 4>;

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

  /// @brief Returns the socket, creating it on first use
  ///
  /// The caller must hold GetMutex(st) while using the socket, see
  /// SocketWithMutex.
  void *GetOrCreateSocket(SocketTypes st);
  bool HasBindsOrConnections(SocketTypes st);

  void SetDebugFlag(bool val);
  bool GetDebugFlag() const;

  void SetRecvBusyWaitingFlag(bool val);
  bool GetRecvBusyWaitingFlag() const;

  void SetInterceptorFlag(bool val);
  bool GetInterceptorFlag() const;

  /// @brief Time zeromq_client_recv() waits for a message in seconds, 0 waits
  /// forever
  void SetClientRecvTimeout(double val);
  double GetClientRecvTimeout() const;

  /// @name Replies to client requests which the caller gave up on
  ///
  /// zeromq_client_recv() returns an empty string after an abort or the
  /// timeout, but the reply can still arrive later. It must not be returned
  /// as the reply to the next request.
  /// @{
  void ClientRequestSent();
  void AbandonClientReplies();
  /// @return true if the reply belongs to a request the caller gave up on
  bool ClientReplyReceived();
  /// @}

  void CloseConnections();
  void AddToListOfBindsOrConnections(const std::string &localPoint,
                                     SocketTypes st);
  std::vector<std::string> GetListOfBindsOrConnections(SocketTypes st);
  ConcurrentQueue<OutputMessagePtr> &GetXOPNoticeQueue();

  std::recursive_mutex &GetMutex(SocketTypes st);

  void SetLoggingFlag(bool val);
  bool GetLoggingFlag() const;

  void AddLogEntry(const json &doc, MessageDirection dir);
  void AddLogEntry(const json &doc, const std::string &identity,
                   MessageDirection dir);
  void AddLogEntry(const std::string &str);
  void AddLogEntry(const std::string &str, MessageDirection dir);
  void AddLogEntry(const std::string &str, const std::string &identity,
                   MessageDirection dir);
  void SetLoggingTemplate(const std::string &loggingTemplate);
  void InitLogging();

  void AddSubscriberMessageFilter(std::string filter);

  void RemoveSubscriberMessageFilter(const std::string &filter);

  void SetInterceptorFunctionName(const std::string &funcName);
  std::string GetInterceptorFunctionName();

private:
  GlobalData();
  ~GlobalData()                             = default;
  GlobalData(const GlobalData &)            = delete;
  GlobalData &operator=(const GlobalData &) = delete;

  struct SocketTypeData
  {
    std::vector<std::string> m_list; // list of connections or binds
    void *m_zmq_socket{nullptr};
    std::recursive_mutex m_mutex;
  };

  bool HasSocket(SocketTypes st);

  SocketTypeData &GetSocketTypeData(SocketTypes st);

  SocketTypeData m_client, m_server, m_pub, m_sub;
  std::recursive_mutex m_settingsMutex;

  std::atomic<bool> m_debugging;
  std::atomic<bool> m_busyWaiting;
  std::atomic<bool> m_logging;
  std::atomic<bool> m_interceptor;
  std::atomic<double> m_clientRecvTimeout{0};
  size_t m_clientPendingReplies{0};   ///< guarded by the client mutex
  size_t m_clientAbandonedReplies{0}; ///< guarded by the client mutex

  ConcurrentQueue<OutputMessagePtr> m_queue;
  std::unique_ptr<Logging> m_loggingSink;
  std::recursive_mutex m_loggingLock;
  void *zmq_context;
  std::vector<std::string> m_subMessageFilters;

  std::recursive_mutex m_interceptorLock;
  std::string m_interceptorFuncName;
};

template <>
struct fmt::formatter<SocketTypes> : fmt::formatter<std::string>
{
  // parse is inherited from formatter<std::string>.
  template <typename FormatContext>
  auto format(const SocketTypes &st, FormatContext &ctx) const
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
    case SocketTypes::Publisher:
      name = "Publisher";
      break;
    case SocketTypes::Subscriber:
      name = "Subscriber";
      break;
    }

    return formatter<std::string>::format(name, ctx);
  }
};
