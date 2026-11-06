#pragma once

#include <exception>
#include <sstream>
#include <string>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

#define ASSERT(A)                                                              \
  if(!(A))                                                                     \
  {                                                                            \
    throw IgorException(                                                       \
        INTERNAL_ERROR,                                                        \
        fmt::format(FMT_STRING("The assertion in {} line {} file {} failed"),  \
                    __func__, __LINE__, __FILE__));                            \
  }

#define ZEROMQ_ASSERT_DETAILS(err)                                             \
  fmt::format(                                                                 \
      FMT_STRING("The zmq library call in {} line {} file {} failed with "     \
                 "errno={}"),                                                  \
      __func__, __LINE__, __FILE__, err)

#define ZEROMQ_ASSERT(A)                                                       \
  if(!(A))                                                                     \
  {                                                                            \
    auto err = zmq_errno();                                                    \
    throw IgorException(                                                       \
        INVALID_ARG,                                                           \
        fmt::format(FMT_STRING("ZeroMQ error: {}"), zmq_strerror(err)),        \
        ZEROMQ_ASSERT_DETAILS(err));                                           \
  }

#define ZEROMQ_ASSERT_ENDPOINT(A, endpoint)                                    \
  if(!(A))                                                                     \
  {                                                                            \
    auto err = zmq_errno();                                                    \
    throw IgorException(INVALID_ARG,                                           \
                        fmt::format(FMT_STRING("ZeroMQ error for {}: {}"),     \
                                    endpoint, zmq_strerror(err)),              \
                        ZEROMQ_ASSERT_DETAILS(err));                           \
  }

class IgorException : public std::exception
{
public:
  explicit IgorException(int errorCode);
  IgorException(int errorCode, const std::string &errorMessage);

  /// @param details Information for debugging only, output with the debug flag
  IgorException(int errorCode, const std::string &errorMessage,
                const std::string &details);

  ~IgorException() override;

  IgorException(const IgorException &)            = default;
  IgorException(IgorException &&)                 = default;
  IgorException &operator=(const IgorException &) = delete;
  IgorException &operator=(IgorException &&)      = delete;

  [[nodiscard]] const char *what() const noexcept override;

  [[nodiscard]] int GetErrorCode() const;

  [[nodiscard]] const char *GetDetails() const noexcept;

  /// Displays the exception if required; gets the return code.
  [[nodiscard]] int HandleException() const;

private:
  const int m_errorCode;
  const std::runtime_error m_message;
  const std::runtime_error m_details;
};

void to_json(json &doc, const IgorException &exception);

int HandleException(const std::exception &exception);

#define BEGIN_OUTER_CATCH                                                      \
  p->result = decltype(p->result)();                                           \
  try                                                                          \
  {

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
