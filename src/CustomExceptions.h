#pragma once

#include <sstream>
#include <exception>
#include <string>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

#define ASSERT(A)                                                              \
  if(!(A))                                                                     \
  {                                                                            \
    throw IgorException(                                                       \
        INTERNAL_ERROR,                                                        \
        fmt::sprintf("The assertion in %s line %d file %s failed\r", __func__, \
                     __LINE__, __FILE__));                                     \
  }

#define ZEROMQ_ASSERT(A)                                                       \
  if(!(A))                                                                     \
  {                                                                            \
    auto err = zmq_errno();                                                    \
    throw IgorException(                                                       \
        INVALID_ARG,                                                           \
        fmt::sprintf("The zmq library call in %s line %d file "                \
                     "%s failed with errno=%d and msg=\"%s\"\r",               \
                     __func__, __LINE__, __FILE__, err, zmq_strerror(err)));   \
  }

class IgorException : public std::exception
{
public:
  const int m_errorCode;
  const std::string m_message;

/// Constructors
// Mark default constructor as deprecated
// - Allows use of default constructor when a custom error code hasn't been
// implemented
// - Compiler warning allows us to find usages later
#ifdef WINIGOR
  __declspec(deprecated("Using default error code.  You should replace this "
                        "with a custom error code")) IgorException();
#endif
  explicit IgorException(int errorCode);
  IgorException(int errorCode, std::string errorMessage);

  virtual ~IgorException() override;

  const char *what() const noexcept override;

  /// Displays the exception if required; gets the return code.
  int HandleException() const;
};

void to_json(json &j, const IgorException &e);

int HandleException(const std::exception &e);

#define BEGIN_OUTER_CATCH                                                      \
  p->result = decltype(p->result)();                                           \
  try                                                                          \
  {                                                                            \
    GlobalData::Instance().EnsureInteropProcFileAvailable();

#define END_OUTER_CATCH                                                        \
  return 0;                                                                    \
  }                                                                            \
  catch(const IgorException &e)                                                \
  {                                                                            \
    return e.HandleException();                                                \
  }                                                                            \
  catch(const std::exception &e)                                               \
  {                                                                            \
    return HandleException(e);                                                 \
  }                                                                            \
  catch(...)                                                                   \
  {                                                                            \
    /* Unhandled exception */                                                  \
    return UNHANDLED_CPP_EXCEPTION;                                            \
  }
