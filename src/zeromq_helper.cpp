#include "zeromq_helper.h"

#include "ZeroMQ.h"

ZeroMQMessage::ZeroMQMessage()
{
  int rc = zmq_msg_init(&msg);
  ZEROMQ_ASSERT(rc == 0);
}

ZeroMQMessage::~ZeroMQMessage()
{
  zmq_msg_close(&msg);
  // ignore error
}

zmq_msg_t* ZeroMQMessage::get()
{
  return &msg;
}
