#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

constexpr char PACKAGE_NAME[] = "ZeroMQ";

void SetSocketDefaults(void *s)
{
  int val = 0;
  auto rc = zmq_setsockopt(s, ZMQ_LINGER, &val, sizeof(val));
  ZEROMQ_ASSERT(rc == 0);

  rc = zmq_setsockopt(s, ZMQ_SNDTIMEO, &val, sizeof(val));
  ZEROMQ_ASSERT(rc == 0);

  rc = zmq_setsockopt(s, ZMQ_RCVTIMEO, &val, sizeof(val));
  ZEROMQ_ASSERT(rc == 0);
}

void SetRouterSocketDefaults(void *s)
{
  int val = 1;
  auto rc = zmq_setsockopt(s, ZMQ_ROUTER_MANDATORY, &val, sizeof(val));
  ZEROMQ_ASSERT(rc == 0);

  int64_t bytes = 1024;
  rc            = zmq_setsockopt(s, ZMQ_MAXMSGSIZE, &bytes, sizeof(bytes));
  ZEROMQ_ASSERT(rc == 0);
}

void SetDealerSocketDefaults(void *s)
{
  const char identity[] = "zeromq xop: dealer";
  auto rc = zmq_setsockopt(s, ZMQ_IDENTITY, &identity, strlen(identity));
  ZEROMQ_ASSERT(rc == 0);
}

} // anonymous namespace

GlobalData::GlobalData()
    : m_debugging(false), m_busyWaiting(true), m_logging(false)
{
  zmq_context = zmq_ctx_new();
  ZEROMQ_ASSERT(zmq_context != nullptr);
}

void *GlobalData::ZMQClientSocket()
{
  if(!zmq_client_socket)
  {
    LockGuard lock(m_clientMutex);

    DEBUG_OUTPUT("Creating client socket");

    zmq_client_socket = zmq_socket(zmq_context, ZMQ_DEALER);
    ZEROMQ_ASSERT(zmq_client_socket != nullptr);

    SetSocketDefaults(zmq_client_socket);
    SetDealerSocketDefaults(zmq_client_socket);
  }

  return zmq_client_socket;
}

bool GlobalData::HasClientSocket()
{
  LockGuard lock(m_clientMutex);

  return zmq_client_socket != nullptr;
}

void *GlobalData::ZMQServerSocket()
{
  if(!zmq_server_socket)
  {
    LockGuard lock(m_serverMutex);

    DEBUG_OUTPUT("Creating server socket");

    zmq_server_socket = zmq_socket(zmq_context, ZMQ_ROUTER);
    ZEROMQ_ASSERT(zmq_server_socket != nullptr);

    SetSocketDefaults(zmq_server_socket);
    SetRouterSocketDefaults(zmq_server_socket);
  }

  return zmq_server_socket;
}

bool GlobalData::HasServerSocket()
{
  LockGuard lock(m_serverMutex);

  return zmq_server_socket != nullptr;
}

void GlobalData::SetDebugFlag(bool val)
{
  LockGuard lock(m_settingsMutex);

  DEBUG_OUTPUT("new value={}", val);
  m_debugging = val;
};

bool GlobalData::GetDebugFlag() const
{
  return m_debugging;
}

void GlobalData::SetRecvBusyWaitingFlag(bool val)
{
  LockGuard lock(m_settingsMutex);

  DEBUG_OUTPUT("new value={}", val);
  m_busyWaiting = val;
}

bool GlobalData::GetRecvBusyWaitingFlag() const
{
  return m_busyWaiting;
}

void GlobalData::CloseConnections()
{
  if(HasClientSocket())
  {
    DEBUG_OUTPUT("Connections={}", m_connections.size());

    try
    {
      // client
      GET_CLIENT_SOCKET(socket);

      for(const auto &conn : m_connections)
      {
        auto rc = zmq_disconnect(socket.get(), conn.c_str());
        DEBUG_OUTPUT("zmq_disconnect({}) returned={}", conn, rc);
        // ignore errors
      }
      m_connections.clear();

      auto rc = zmq_close(socket.get());
      ZEROMQ_ASSERT(rc == 0);
      zmq_client_socket = nullptr;
    }
    catch(...)
    {
      // ignore errors
    }
  }

  if(HasServerSocket())
  {
    DEBUG_OUTPUT("Binds={}", m_binds.size());

    try
    {
      // server
      GET_SERVER_SOCKET(socket);

      for(const auto &bind : m_binds)
      {
        auto rc = zmq_unbind(socket.get(), bind.c_str());
        DEBUG_OUTPUT("zmq_unbind({}) returned={}", bind, rc);
        // ignore errors
      }
      m_binds.clear();

      auto rc = zmq_close(socket.get());
      ZEROMQ_ASSERT(rc == 0);
      zmq_server_socket = nullptr;
    }
    catch(...)
    {
      // ignore errors
    }
  }
}

bool GlobalData::HasBinds()
{
  LockGuard lock(m_serverMutex);

  return !m_binds.empty();
}

void GlobalData::AddToListOfBinds(const std::string &localPoint)
{
  LockGuard lock(m_serverMutex);

  m_binds.push_back(localPoint);
}

bool GlobalData::HasConnections()
{
  LockGuard lock(m_clientMutex);

  return !m_connections.empty();
}

void GlobalData::AddToListOfConnections(const std::string &remotePoint)
{
  LockGuard lock(m_clientMutex);

  m_connections.push_back(remotePoint);
}

ConcurrentQueue<OutputMessagePtr> &GlobalData::GetXOPNoticeQueue()
{
  return m_queue;
}

void GlobalData::SetLoggingFlag(bool val)
{
  m_logging = val;
}

bool GlobalData::GetLoggingFlag() const
{
  return m_logging;
}

void GlobalData::AddLogEntry(const json &doc, MessageDirection dir)
{
  if(!GetLoggingFlag())
  {
    return;
  }

  LockGuard lock(m_loggingLock);

  m_loggingSink->AddLogEntry(doc, dir);
}

void GlobalData::AddLogEntry(const std::string &str, MessageDirection dir)
{
  if(!GetLoggingFlag())
  {
    return;
  }

  LockGuard lock(m_loggingLock);

  m_loggingSink->AddLogEntry(str, dir);
}

void GlobalData::SetLoggingTemplate(const std::string &loggingTemplate)
{
  LockGuard lock(m_loggingLock);

  m_loggingSink = std::make_unique<Logging>(PACKAGE_NAME, loggingTemplate);
}

void GlobalData::InitLogging()
{
  LockGuard lock(m_loggingLock);

  m_loggingSink = std::make_unique<Logging>(PACKAGE_NAME);
}
