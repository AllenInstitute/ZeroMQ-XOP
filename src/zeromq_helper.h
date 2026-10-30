#pragma once

#include <memory>
#include <vector>

#include <zmq.h>

// RAII class for zmq_msg_t
class ZeroMQMessage
{
public:
  ZeroMQMessage();
  ~ZeroMQMessage();

  zmq_msg_t *get();

  ZeroMQMessage(const ZeroMQMessage &)            = delete;
  ZeroMQMessage(ZeroMQMessage &&)                 = delete;
  ZeroMQMessage &operator=(const ZeroMQMessage &) = delete;
  ZeroMQMessage &operator=(ZeroMQMessage &&)      = delete;

private:
  zmq_msg_t msg{};
};
