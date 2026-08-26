#include "ZeroMQ.h"
#include "MessageHandler.h"
#include "MessageHandlerPauseGuard.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

MessageHandlerPauseGuard::MessageHandlerPauseGuard(bool enable)
    : m_wasRunning(enable && MessageHandler::Instance().IsRunning())
{
  if(m_wasRunning)
  {
    MessageHandler::Instance().Stop();
  }
}

MessageHandlerPauseGuard::~MessageHandlerPauseGuard()
{
  if(m_wasRunning)
  {
    try
    {
      MessageHandler::Instance().Start();
    }
    catch(...)
    {
      EMERGENCY_OUTPUT("Could not restart the message handler, call "
                       "zeromq_handler_start() to restart it manually.");
    }
  }
}
