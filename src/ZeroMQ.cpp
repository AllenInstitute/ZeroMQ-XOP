#include "ZeroMQ.h"
#include "MessageHandler.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

bool idleInProgress = false;

} // anonymous namespace

/*	XOPEntry()
  This is the entry point from the host application to the XOP for all messages
  after the INIT message.
*/
extern "C" void XOPEntry()
{
  try
  {
    switch(GetXOPMessage())
    {
    case FUNCADDRS:
    {
      auto result = RegisterFunction();
      SetXOPResult(result);
    }
    break;
    case IDLE:
      if(!idleInProgress)
      {
        idleInProgress = true;
        MessageHandler::Instance().HandleAllQueuedMessages();
        OutputQueuedNotices();
        idleInProgress = false;
      }
      break;
    case CLEANUP:
      MessageHandler::Instance().Stop();
      HeartbeatPublisher::Instance().Stop();
      GlobalData::Instance().CloseConnections();
      break;
    }
  }
  catch(...)
  {
    EMERGENCY_OUTPUT("Caught exception. This must NOT happen!");
  }
}

/*	XOPMain(ioRecHandle)

  This is the initial entry point at which the host application calls XOP.
  The message sent by the host must be INIT.

  XOPMain does any necessary initialization and then sets the XOPEntry field of
  the ioRecHandle to the address to be called for future messages.
*/

HOST_IMPORT int XOPMain(IORecHandle ioRecHandle)
{
  try
  {
    XOPInit(ioRecHandle);  // Do standard XOP initialization
    SetXOPEntry(XOPEntry); // Set entry point for future calls

    SetXOPType(RESIDENT | IDLE);

    if(igorVersion < XOP_MINIMUM_IGORVERSION)
    {
      SetXOPResult(OLD_IGOR);
      return EXIT_FAILURE;
    }

    GlobalData::Instance();

    // initialize logging, this needs to be done after creating
    // GlobalData, as we need to be able to output debug messages for that
    GlobalData::Instance().InitLogging();

    HeartbeatPublisher::Instance().Start();

#ifdef _DEBUG
    ApplyFlags(ZeroMQ_SET_FLAGS::DEBUG);
#endif // _DEBUG

    SetXOPResult(EXIT_SUCCESS);
    return EXIT_SUCCESS;
  }
  catch(const IgorException &e)
  {
    SetXOPResult(e.GetErrorCode());
    return EXIT_FAILURE;
  }
  catch(...)
  {
    SetXOPResult(UNHANDLED_CPP_EXCEPTION);
    return EXIT_FAILURE;
  }
}
