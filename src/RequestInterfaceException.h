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

  RequestInterfaceException(const RequestInterfaceException &) = default;
  RequestInterfaceException(RequestInterfaceException &&)      = default;
  RequestInterfaceException &
  operator=(const RequestInterfaceException &)                       = default;
  RequestInterfaceException &operator=(RequestInterfaceException &&) = default;
};

#endif // REQUEST_INTERFACE_EXCEPTION_H
