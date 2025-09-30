#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

constexpr char PACKAGE_NAME[] = "ZeroMQ";

void ApplySocketDefaults(void *s, SocketTypes st)
{
  int valZero = 0;
  int valOne  = 1;
  int rc      = 0;

  rc = zmq_setsockopt(s, ZMQ_LINGER, &valZero, sizeof(valZero));
  ZEROMQ_ASSERT(rc == 0);

  rc = zmq_setsockopt(s, ZMQ_SNDTIMEO, &valZero, sizeof(valZero));
  ZEROMQ_ASSERT(rc == 0);

  rc = zmq_setsockopt(s, ZMQ_RCVTIMEO, &valOne, sizeof(valOne));
  ZEROMQ_ASSERT(rc == 0);

  switch(st)
  {
  case SocketTypes::Client:
  {
    const char identity[] = "zeromq xop: dealer";
    rc = zmq_setsockopt(s, ZMQ_IDENTITY, &identity, strlen(identity));
    ZEROMQ_ASSERT(rc == 0);
    return;
  }
  case SocketTypes::Publisher:
    // do nothing
    return;
  case SocketTypes::Server:
  {
    rc = zmq_setsockopt(s, ZMQ_ROUTER_MANDATORY, &valOne, sizeof(valOne));
    ZEROMQ_ASSERT(rc == 0);

    int64_t bytes = 1024;

    rc = zmq_setsockopt(s, ZMQ_MAXMSGSIZE, &bytes, sizeof(bytes));
    ZEROMQ_ASSERT(rc == 0);
    return;
  }
  case SocketTypes::Subscriber:
    // do nothing
    return;
  }

  ASSERT(0);
}

int GetZeroMQSocketConstant(SocketTypes st)
{
  switch(st)
  {
  case SocketTypes::Client:
    return ZMQ_DEALER;
  case SocketTypes::Publisher:
    return ZMQ_PUB;
  case SocketTypes::Server:
    return ZMQ_ROUTER;
  case SocketTypes::Subscriber:
    return ZMQ_SUB;
  }

  ASSERT(0);
}
} // anonymous namespace

AllSocketTypesArray GetAllSocketTypes()
{
  return AllSocketTypesArray{SocketTypes::Client, SocketTypes::Subscriber,
                             SocketTypes::Publisher, SocketTypes::Server};
}

GlobalData::GlobalData()
    : m_debugging(false), m_busyWaiting(true), m_logging(false)
{
  zmq_context = zmq_ctx_new();
  ZEROMQ_ASSERT(zmq_context != nullptr);
}

void *GlobalData::ZMQSocket(SocketTypes st)
{
  LockGuard lock(GetMutex(st));

  auto &socketData = GetSocketTypeData(st);
  void *&socket    = socketData.m_zmq_socket;

  if(socket)
  {
    return socket;
  }

  socket = zmq_socket(zmq_context, GetZeroMQSocketConstant(st));
  ZEROMQ_ASSERT(socket != nullptr);

  DEBUG_OUTPUT("Creating {} socket {}", st, socket);

  ApplySocketDefaults(socket, st);

  return socket;
}

bool GlobalData::HasBindsOrConnections(SocketTypes st)
{
  LockGuard lock(GetMutex(st));

  auto &socketData = GetSocketTypeData(st);

  return !socketData.m_list.empty();
}

bool GlobalData::HasSocket(SocketTypes st)
{
  LockGuard lock(GetMutex(st));

  auto &socketData = GetSocketTypeData(st);
  return socketData.m_zmq_socket != nullptr;
}

void GlobalData::SetDebugFlag(bool val)
{
  LockGuard lock(m_settingsMutex);

  DEBUG_OUTPUT("new value={}", val);
  m_debugging = val;
}

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
  if(HasSocket(SocketTypes::Subscriber))
  {
    RemoveSubscriberMessageFilter("");
  }

  for(auto st : GetAllSocketTypes())
  {
    LockGuard lock(GetMutex(st));

    if(!HasSocket(st))
    {
      continue;
    }

    auto &socketData = GetSocketTypeData(st);
    auto &list       = socketData.m_list;

    DEBUG_OUTPUT("SocketType {}, Connections={}", st, list.size());

    try
    {
      GET_SOCKET(socket, st);

      for(const auto &conn : list)
      {
        int rc = 0;
        switch(st)
        {
        case SocketTypes::Server:
        case SocketTypes::Publisher:
          rc = zmq_unbind(socket.get(), conn.c_str());
          DEBUG_OUTPUT("zmq_disconnect({}) returned={}", conn, rc);

          break;
        case SocketTypes::Client:
        case SocketTypes::Subscriber:
          rc = zmq_disconnect(socket.get(), conn.c_str());
          DEBUG_OUTPUT("zmq_unbind({}) returned={}", conn, rc);
          break;
        }
        // ignore errors
      }
      list.clear();

      auto rc = zmq_close(socket.get());
      ZEROMQ_ASSERT(rc == 0);
      socketData.m_zmq_socket = nullptr;
    }
    catch(...)
    {
      // ignore errors
    }
  }
}

void GlobalData::AddToListOfBindsOrConnections(const std::string &point,
                                               SocketTypes st)
{
  LockGuard lock(GetMutex(st));

  auto &socketData = GetSocketTypeData(st);
  socketData.m_list.push_back(point);
}

ConcurrentQueue<OutputMessagePtr> &GlobalData::GetXOPNoticeQueue()
{
  return m_queue;
}

void GlobalData::SetLoggingFlag(bool val)
{
  LockGuard lock(m_settingsMutex);

  DEBUG_OUTPUT("new value={}", val);

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

void GlobalData::AddSubscriberMessageFilter(std::string filter)
{
  DEBUG_OUTPUT("filter={}", filter);

  const auto st = SocketTypes::Subscriber;

  GET_SOCKET(socket, st);

  auto it = std::find(std::begin(m_subMessageFilters),
                      std::end(m_subMessageFilters), filter);

  if(it != std::end(m_subMessageFilters))
  {
    throw IgorException(MESSAGE_FILTER_DUPLICATED);
  }

  auto rc = zmq_setsockopt(socket.get(), ZMQ_SUBSCRIBE, filter.c_str(),
                           filter.size());
  ZEROMQ_ASSERT(rc == 0);

  m_subMessageFilters.emplace_back(std::move(filter));
}

void GlobalData::RemoveSubscriberMessageFilter(const std::string &filter)
{
  DEBUG_OUTPUT("filter={}", filter);

  const auto st = SocketTypes::Subscriber;

  GET_SOCKET(socket, st);

  auto unsubscribe = [&socket](const std::string &entry) -> void
  {
    auto rc = zmq_setsockopt(socket.get(), ZMQ_UNSUBSCRIBE, entry.c_str(),
                             entry.size());
    ZEROMQ_ASSERT(rc == 0);
  };

  if(filter.empty())
  {
    for(auto &entry : m_subMessageFilters)
    {
      unsubscribe(entry);
    }
    m_subMessageFilters.clear();
  }
  else
  {
    auto it = std::find(std::begin(m_subMessageFilters),
                        std::end(m_subMessageFilters), filter);

    if(it == std::end(m_subMessageFilters))
    {
      throw IgorException(MESSAGE_FILTER_MISSING);
    }

    unsubscribe(filter);
    m_subMessageFilters.erase(it);
  }
}

std::recursive_mutex &GlobalData::GetMutex(SocketTypes st)
{
  switch(st)
  {
  case SocketTypes::Client:
    return m_client.m_mutex;
  case SocketTypes::Publisher:
    return m_pub.m_mutex;
  case SocketTypes::Server:
    return m_server.m_mutex;
  case SocketTypes::Subscriber:
    return m_sub.m_mutex;
  }

  ASSERT(0);
}

GlobalData::SocketTypeData &GlobalData::GetSocketTypeData(SocketTypes st)
{
  switch(st)
  {
  case SocketTypes::Client:
    return m_client;
  case SocketTypes::Publisher:
    return m_pub;
  case SocketTypes::Server:
    return m_server;
  case SocketTypes::Subscriber:
    return m_sub;
  }

  ASSERT(0);
}
