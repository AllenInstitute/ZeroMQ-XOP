#ifndef CUSTOM_EXCEPTIONS_H
#define CUSTOM_EXCEPTIONS_H

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
        fmt::format(                                                           \
            FMT_STRING("The assertion in {} line {} file {} failed\r"),        \
            __func__, __LINE__, __FILE__));                                    \
  }

#define ZEROMQ_ASSERT(A)                                                       \
  if(!(A))                                                                     \
  {                                                                            \
    auto err = zmq_errno();                                                    \
    throw IgorException(                                                       \
        INVALID_ARG,                                                           \
        fmt::format(FMT_STRING("The zmq library call in {} line {} file "      \
                               "{} failed with errno={} and msg=\"{}\"\r"),    \
                    __func__, __LINE__, __FILE__, err, zmq_strerror(err)));    \
  }

class IgorException : public std::exception
{
public:
  explicit IgorException(int errorCode);
  IgorException(int errorCode, const std::string &errorMessage);

  ~IgorException() override;

  IgorException(const IgorException &)            = default;
  IgorException(IgorException &&)                 = default;
  IgorException &operator=(const IgorException &) = default;
  IgorException &operator=(IgorException &&)      = default;

  [[nodiscard]] const char *what() const noexcept override;

  [[nodiscard]] int GetErrorCode() const;

  /// Displays the exception if required; gets the return code.
  [[nodiscard]] int HandleException() const;

private:
  int m_errorCode;
  std::runtime_error m_message;
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

#endif // CUSTOM_EXCEPTIONS_H
