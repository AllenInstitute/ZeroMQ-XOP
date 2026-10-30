#pragma once

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
  operator=(const RequestInterfaceException &)                       = delete;
  RequestInterfaceException &operator=(RequestInterfaceException &&) = delete;
};
