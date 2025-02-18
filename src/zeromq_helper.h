#pragma once

#include <vector>
#include <memory>

#include <zmq.h>

// RAII class for zmq_msg_t
class ZeroMQMessage
{
public:
  ZeroMQMessage();
  ~ZeroMQMessage();

  zmq_msg_t* get();

  ZeroMQMessage(const ZeroMQMessage &) = delete;
  void operator=(const ZeroMQMessage &) = delete;

private:
  zmq_msg_t msg;
};
