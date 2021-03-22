#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

std::string GetErrorMessageFromCode(int errorCode)
{
  switch(errorCode)
  {
  case REQ_UNKNOWN_ERROR:
    return "Unknown error in request interface.";
  case REQ_INVALID_JSON_OBJECT:
    return "The string is not a valid json object.";
  case REQ_INVALID_VERSION:
    return "Request interface version is missing or invalid.";
  case REQ_INVALID_OPERATION:
    return "Unknown operation type.";
  case REQ_INVALID_OPERATION_FORMAT:
    return "Invalid operation format.";
  case REQ_INVALID_MESSAGEID:
    return "Invalid optional messageID.";
  case REQ_OUT_OF_MEMORY:
    return "Request cancelled due to Out Of Memory condition.";
  case REQ_NON_EXISTING_FUNCTION:
    return "CallFunction: Unknown function.";
  case REQ_PROC_NOT_COMPILED:
    return "Procedures are not compiled.";
  case REQ_TOO_FEW_FUNCTION_PARAMS:
    return "CallFunction: Too few parameters.";
  case REQ_TOO_MANY_FUNCTION_PARAMS:
    return "CallFunction: Too many parameters.";
  case REQ_UNSUPPORTED_FUNC_SIG:
    return "CallFunction: Unsupported function signature.";
  case REQ_UNSUPPORTED_FUNC_RET:
    return "CallFunction: Unsupported function return type.";
  case REQ_INVALID_PARAM_FORMAT:
    return "CallFunction: Parameter is not an array or otherwise invalid.";
  case REQ_FUNCTION_ABORTED:
    return "CallFunction: The function was partially executed but aborted at "
           "some point.";
  default:
    ASSERT(0);
  }
}
} // anonymous namespace

RequestInterfaceException::RequestInterfaceException(int errorCode)
    : IgorException(errorCode, GetErrorMessageFromCode(errorCode))
{
}

RequestInterfaceException::~RequestInterfaceException()
{
  // do nothing
}
