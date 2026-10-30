#ifndef REQUEST_INTERFACE_EXCEPTION_H
#define REQUEST_INTERFACE_EXCEPTION_H

#include "CustomExceptions.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class RequestInterfaceException : public IgorException
{
public:
  explicit RequestInterfaceException(int errorCode);
  ~RequestInterfaceException() override;
};

#endif // REQUEST_INTERFACE_EXCEPTION_H
