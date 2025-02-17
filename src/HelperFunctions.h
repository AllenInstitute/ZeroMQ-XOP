#pragma once

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

#include <sstream>
#include <iomanip>

#ifdef MACIGOR
#include <sys/stat.h>
#endif

#include "Errors.h"
#include "send_struct.h"

#ifdef MACIGOR64
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#endif

#include "SafeInt/SafeInt.hpp"

#ifdef MACIGOR64
#pragma clang diagnostic pop
#endif

bool IsBitSet(int val, int bit);
int ClearBit(int val, int bit);
int SetBit(int val, int bit);

const int DEFAULT_INDENT = 4;

/// @brief Safe type conversion
template <typename T, typename U>
T To(U val)
{
  T result;

  if(!SafeCast(val, result))
  {
    throw IgorException(PNTS_INCOMPATIBLE);
  }

  return result;
}

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

std::string GetStringFromHandleWithDispose(Handle strHandle);

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
int ZeroMQPublisherSend(const SendStorageVec &vec);
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

template <typename... T>
void xop_logging(OutputMode mode, const char *func, int line,
                 fmt::format_string<T...> format, T &&...args)
{
  vlog(mode, func, line, format, fmt::make_format_args(args...));
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

Handle GetHandleFromString(const std::string &str);

/// @brief Sets an element of a wave. Supports numeric, text, Int64, UInt64
/// waves (Not WaveRef and DatafolderRef)
///        The template type must fit the wave type.
/// @param[in] w handle of target wave
/// @param[in] dims IndexInt vector of MAX_DIMENSIONS size that points to the
/// elements location within the wave.
/// @param[in] value value that should be written to the location given by dims
/// in w
template <typename T,
          typename std::enable_if_t<std::is_same<DataFolderHandle, T>::value || std::is_same<waveHndl, T>::value ||
                                        std::is_same<std::string, T>::value || std::is_same<float, T>::value ||
                                        std::is_same<double, T>::value || std::is_integral<T>::value,
                                    int> = 0>
void SetWaveElement(waveHndl w, std::vector<IndexInt> &dims, const T &value)
{
  int err;

  if(!w)
  {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS);

  const int type = WaveType(w);
  if(((type == NT_FP32) && std::is_same<float, T>::value) || ((type == NT_FP64) && std::is_same<double, T>::value) ||
     ((type & NT_I8) && std::is_integral<T>::value && (sizeof(T) == sizeof(int8_t))) ||
     ((type & NT_I16) && std::is_integral<T>::value && (sizeof(T) == sizeof(int16_t))) ||
     ((type & NT_I32) && std::is_integral<T>::value && (sizeof(T) == sizeof(int32_t))))

  {
    std::array<double, 2> v;
    v[0] = static_cast<double>(value);
    if(err = MDSetNumericWavePointValue(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error writing values to numeric wave");
    }
  }
  else if((type == NT_I64) && std::is_integral<T>::value && (sizeof(T) == sizeof(int64_t)) && std::is_signed<T>::value)
  {
    std::array<SInt64, 2> v;
    v[0] = static_cast<SInt64>(value);
    if(err = MDSetNumericWavePointValueSInt64(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error writing values to INT64 wave");
    }
  }
  else if((type == (NT_I64 | NT_UNSIGNED)) && std::is_integral<T>::value && (sizeof(T) == sizeof(uint64_t)) &&
          std::is_unsigned<T>::value)
  {
    std::array<UInt64, 2> v;
    v[0] = static_cast<UInt64>(value);
    if(err = MDSetNumericWavePointValueUInt64(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error writing values to UINT64 wave");
    }
  }
  else
  {
    throw IgorException(ERR_INVALID_TYPE, "XOP Bug: Unsupported wave type or wave type and input "
                                          "type are not the same.");
  }
}

template <>
void SetWaveElement<std::string>(waveHndl w, std::vector<IndexInt> &dims, const std::string &value);

template <>
void SetWaveElement<waveHndl>(waveHndl w, std::vector<IndexInt> &dims, const waveHndl &value);

template <>
void SetWaveElement<DataFolderHandle>(waveHndl w, std::vector<IndexInt> &dims, const DataFolderHandle &value);

/// @brief Gets an element of a wave. Supports numeric, text, Int64, UInt64
/// waves (Not WaveRef and DatafolderRef)
///        The template type must fit the wave type.
/// @param[in] w handle of target wave
/// @param[in] dims IndexInt vector of MAX_DIMENSIONS size that points to the
/// elements location within the wave.
/// @return value value that is read at the location given by dims in w
template <typename T,
          typename std::enable_if_t<std::is_same<DataFolderHandle, T>::value || std::is_same<waveHndl, T>::value ||
                                        std::is_same<std::string, T>::value || std::is_same<float, T>::value ||
                                        std::is_same<double, T>::value || std::is_integral<T>::value,
                                    int> = 0>
T GetWaveElement(waveHndl w, std::vector<IndexInt> &dims)
{
  int err;

  if(!w)
  {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS);

  const int type = WaveType(w);
  if(((type == NT_FP32) && std::is_same<float, T>::value) || ((type == NT_FP64) && std::is_same<double, T>::value) ||
     ((type & NT_I8) && std::is_integral<T>::value && (sizeof(T) == sizeof(int8_t))) ||
     ((type & NT_I16) && std::is_integral<T>::value && (sizeof(T) == sizeof(int16_t))) ||
     ((type & NT_I32) && std::is_integral<T>::value && (sizeof(T) == sizeof(int32_t))))

  {
    std::array<double, 2> v;
    if(err = MDGetNumericWavePointValue(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error reading values from numeric wave");
    }
    return static_cast<T>(v[0]);
  }
  if((type == NT_I64) && std::is_integral<T>::value && (sizeof(T) == sizeof(int64_t)) && std::is_signed<T>::value)
  {
    std::array<SInt64, 2> v;
    if(err = MDGetNumericWavePointValueSInt64(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error reading values from INT64 wave");
    }
    return static_cast<T>(v[0]);
  }
  if((type == (NT_I64 | NT_UNSIGNED)) && std::is_integral<T>::value && (sizeof(T) == sizeof(uint64_t)) &&
     std::is_unsigned<T>::value)
  {
    std::array<UInt64, 2> v;
    if(err = MDGetNumericWavePointValueUInt64(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error reading values from UINT64 wave");
    }
    return static_cast<T>(v[0]);
  }
  throw IgorException(ERR_INVALID_TYPE, "XOP Bug: Unsupported wave type or wave type and "
                                        "template type are not the same.");
}

template <>
std::string GetWaveElement<std::string>(waveHndl w, std::vector<IndexInt> &dims);

template <>
waveHndl GetWaveElement<waveHndl>(waveHndl w, std::vector<IndexInt> &dims);

template <>
DataFolderHandle GetWaveElement<DataFolderHandle>(waveHndl w, std::vector<IndexInt> &dims);

/// @brief Compares expected wave dimensions vs actual wave dimensions. Throws
/// an IgorException when not equal.
/// @param[in] w wave handle of wave
/// @param[in] expectedDims vector with expected dimension sizes, only up to
/// MAX_DIMENSIONS entries will be considered
/// @param[in] errorMsg string containing a custom error message
void CheckWaveDimension(waveHndl w, const std::vector<CountInt> &expectedDims, const std::string &errorMsg);

template <typename T>
T *GetWaveDataPtr(waveHndl waveH)
{
  BCInt dataOffset     = 0;
  const int accessMode = kMDWaveAccessMode0;
  const int ret = MDAccessNumericWaveData(waveH, accessMode, &dataOffset);

  if(ret != 0)
  {
    throw std::runtime_error(
        fmt::format("MDAccessNumericWaveData returned error {}", ret));
  }

  return reinterpret_cast<T *>(reinterpret_cast<char *>(*waveH) + dataOffset);
}

/// @brief Retrieves dimensions of a wave.
/// @param[in] w wave handle of wave
/// @return vector with size of each dimension. The vector has size
/// MAX_DIMENSIONS + 1 and the first unused dimension is set to size 0
std::vector<CountInt> GetWaveDimension(waveHndl w);

/// @brief Retrieves dimensions and number of dimensions of a wave.
/// @param[in] w wave handle of wave
/// @param[out] numDims number of dimensions
/// @return vector with size of each dimension. The vector has size
/// MAX_DIMENSIONS + 1.
std::vector<CountInt> GetWaveDimension(waveHndl w, int &numDims);
