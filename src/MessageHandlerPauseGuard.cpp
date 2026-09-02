#include "MessageHandlerPauseGuard.h"

#include "MessageHandler.h"

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
      // Best effort: The caller can always call zeromq_handler_start()
    }
  }
}
