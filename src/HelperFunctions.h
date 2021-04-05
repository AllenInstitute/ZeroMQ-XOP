#pragma once

#include <sstream>
#include <iomanip>

#ifdef MACIGOR
#include <sys/stat.h>
#endif

#include "Errors.h"

bool IsBitSet(int val, int bit);
int ClearBit(int val, int bit);
int SetBit(int val, int bit);

const int DEFAULT_INDENT = 4;

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

/// @brief Converts a double value to a specified integer type.
///
/// Returns an error if:
/// - The value is NaN of +/- inf
/// - The value lies outside the range of the integer representation.
///
/// The value is truncated towards zero
/// That is:
/// - Positive numbers are rounded down
/// - Negative numbers are rounded up
///
/// If the value is NaN, zero is returned.
///
/// @tparam	T	integer type to convert to.
/// @param	val	value to convert
/// @return	converted value
template <typename T>
T lockToIntegerRange(double val)
{
  // If value is NaN or inf, return an appropriate error.
  if(std::isnan(val) || std::isinf(val))
  {
    throw IgorException(kDoesNotSupportNaNorINF);
  }

  // If value lies outside range of integer type, return an error.
  if(val > static_cast<double>(std::numeric_limits<T>::max()) ||
     val < static_cast<double>(std::numeric_limits<T>::min()))
  {
    throw IgorException(kParameterOutOfRange);
  }

  // Truncate towards zero.
  // 10.1 becomes 10
  // -10.1 becomes -10.
  if(val > 0)
  {
    val = std::floor(val);
  }
  if(val < 0)
  {
    val = std::ceil(val);
  }

  return static_cast<T>(val);
}

template <>
bool lockToIntegerRange<bool>(double val);

/// @brief Return the size in bytes of the given Igor Pro wave types
///
/// The returned size is zero for non-numeric wave types
std::size_t GetWaveElementSize(int dataType);

/// @brief Set all elements of the given wave to zero
///
/// Does nothing for non-numeric waves
void WaveClear(waveHndl wv);

/// @brief Convert Igor string into std::string
///
/// If the string handle is null, the empty string is returned.
///
/// @param	strHandle	handle to Igor string
/// @return	std::string containing the same data
std::string GetStringFromHandle(Handle strHandle);

void SetDimensionLabels(waveHndl h, int Dimension,
                        const std::vector<std::string> &dimLabels);

void ApplyFlags(double flags);

namespace ZeroMQ_SET_FLAGS
{

enum ZeroMQ_SET_FLAGS
{
  DEFAULT              = 1,
  DEBUG                = 2,
  IPV6                 = 4,
  NO_RECV_BUSY_WAITING = 8,
  LOGGING              = 16
};
}

std::string GetLastEndPoint(void *s);
void ToggleIPV6Support(bool enable);

template <typename T, int withComma>
struct GetFormatString
{
  std::string operator()() const
  {
    if(withComma)
    {
      return "{}, ";
    }

    return "{}";
  }
};

template <int withComma>
struct GetFormatString<double, withComma>
{
  std::string operator()() const
  {
    static_assert(std::numeric_limits<double>::digits10 == 15,
                  "Unexpected double precision");

    if(withComma)
    {
      return "{:.15g}, ";
    }

    return "{:.15g}";
  }
};

template <typename T>
std::string To_stringHighRes(const T val)
{
  std::string fmt = GetFormatString<T, 0>()();

  return fmt::format(fmt, val);
}

double ConvertStringToDouble(const std::string &str);
json CallIgorFunctionFromMessage(const std::string &msg);
json CallIgorFunctionFromReqInterface(const RequestInterfacePtr &req);

int ZeroMQClientSend(const std::string &payload);
int ZeroMQPublisherSend(const std::string &filter, const std::string &payload);
int ZeroMQServerSend(const std::string &identity, const std::string &payload);
int ZeroMQClientReceive(zmq_msg_t *payloadMsg);
int ZeroMQSubscriberReceive(zmq_msg_t *filterMsg, zmq_msg_t *payloadMsg);
int ZeroMQServerReceive(zmq_msg_t *identityMsg, zmq_msg_t *payloadMsg);

std::string SerializeDataFolder(DataFolderHandle dataFolderHandle);
DataFolderHandle DeSerializeDataFolder(const std::string &path);

std::string CreateStringFromZMsg(zmq_msg_t *msg);

void InitHandle(Handle *handle, size_t size);
void WriteZMsgIntoHandle(Handle *handle, zmq_msg_t *msg);

bool IsConvertibleToDouble(const std::string &str);
bool IsWaveType(int igorType);

bool UsesMultipleReturnValueSyntax(const FunctionInfo &fip);
int GetNumberOfReturnValues(const FunctionInfo &fip);
int GetNumberOfInputParameters(const FunctionInfo &fip, int numReturnValues);
int GetFirstInputParameterIndex(const FunctionInfo &fip, int numReturnValues);
std::string CleanupString(std::string str);

// Straight from the fmt documentation
// https://fmt.dev/latest/api.html#argument-lists

enum class OutputMode
{
  Debug,
  Emergency,
  Normal
};

void vlog(OutputMode mode, const char *func, int line, fmt::string_view format,
          fmt::format_args args);

template <typename S, typename... Args>
void xop_logging(OutputMode mode, const char *func, int line, const S &format,
                 Args &&...args)
{
  vlog(mode, func, line, format,
       fmt::make_args_checked<Args...>(format, args...));
}

#define EMERGENCY_OUTPUT(format, ...)                                          \
  xop_logging(OutputMode::Emergency, __func__, __LINE__, FMT_STRING(format),   \
              ##__VA_ARGS__)

#define DEBUG_OUTPUT(format, ...)                                              \
  xop_logging(OutputMode::Debug, __func__, __LINE__, FMT_STRING(format),       \
              ##__VA_ARGS__)

#define NORMAL_OUTPUT(format, ...)                                             \
  xop_logging(OutputMode::Normal, __func__, __LINE__, FMT_STRING(format),      \
              ##__VA_ARGS__)

int CreateDirectory(const std::string &path);

void EnsureDirectoryExists(const std::string &path);

bool IsFreeWave(waveHndl wv);
void DoBindOrConnect(Handle &h, SocketTypes st);
