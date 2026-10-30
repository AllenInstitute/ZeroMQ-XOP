#include "ZeroMQ.h"
#include "HelperFunctions.h"
#include "MessageHandler.h"
#include "MessageHandlerPauseGuard.h"
#include "RequestInterface.h"

namespace
{

constexpr size_t MAX_ENDPOINT_LENGTH = 256;

ExperimentModification MapOutputModeToExperimentModification(OutputMode mode)
{
  switch(mode)
  {
  case OutputMode::Debug:
    return ExperimentModification::Silent;
  case OutputMode::Emergency:
  case OutputMode::Normal:
    return ExperimentModification::Normal;
  }

  ASSERT(0);
}

} // anonymous namespace

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

std::string GetStringFromHandle(Handle strHandle)
{
  // Check for special case of null handle.
  if(strHandle == nullptr)
  {
    return "";
  }

  const size_t strLen = WMGetHandleSize(strHandle);
  return std::string(*strHandle, strLen);
}

/// @brief Returns a string from an Igor string Handle and disposes the Handle
///
/// Returns a string from an Igor string Handle
/// returns an empty string if the sring handle is nullptr
/// use e.g. for FUNCTION parameters (ownership by XOP)
///
/// @param strHandle Igor string handle
/// @return string with the content of the Igor string Handle
std::string GetStringFromHandleWithDispose(Handle strHandle)
{
  auto str = GetStringFromHandle(strHandle);
  WMDisposeHandle(strHandle);

  return str;
}

/// @brief Set dimension labels on wave
///
/// @param	waveHandle	Wave to set dimension labels on
/// @param	Dimension	Dimension to set labels on
/// @param	dimLabels	vector of labels to set
///
/// dimLabels[k] will be assigned to index k of the wave
///
/// @throws Igor error code
void SetDimensionLabels(waveHndl waveHandle, int Dimension,
                        const std::vector<std::string> &dimLabels)
{
  // Check wave exists.
  if(waveHandle == nullptr)
  {
    throw IgorException(NULL_WAVE_OP);
  }

  for(size_t k = 0; k < dimLabels.size(); k++)
  {
    if(dimLabels.at(k).empty())
    {
      // Empty string.  Skip.
      continue;
    }

    if(const int RetVal = MDSetDimensionLabel(waveHandle, Dimension, k,
                                              dimLabels.at(k).c_str()))
    {
      throw IgorException(RetVal);
    }
  }
}

bool IsBitSet(int val, int bit)
{
  const auto ubit = static_cast<unsigned int>(bit);

  return (static_cast<unsigned int>(val) & ubit) == ubit;
}

int ClearBit(int val, int bit)
{
  return static_cast<int>(static_cast<unsigned int>(val) &
                          ~static_cast<unsigned int>(bit));
}

int SetBit(int val, int bit)
{
  return static_cast<int>(static_cast<unsigned int>(val) |
                          static_cast<unsigned int>(bit));
}

template <>
bool lockToIntegerRange<bool>(double val)
{
  // If value is NaN or inf, return an appropriate error.
  if(std::isnan(val) || std::isinf(val))
  {
    throw IgorException(kDoesNotSupportNaNorINF);
  }

  return std::abs(val) > 1e-8;
}

std::size_t GetWaveElementSize(int dataType)
{
  switch(dataType)
  {
  /// size assumptions about real/double values are
  // not guaranteed by the standard but should work
  case NT_CMPLX | NT_FP32:
    return 2 * sizeof(float);
  case NT_CMPLX | NT_FP64:
    return 2 * sizeof(double);
  case NT_FP32:
    return sizeof(float);
  case NT_FP64:
    return sizeof(double);
  case NT_I8:
    return sizeof(int8_t);
  case NT_I8 | NT_UNSIGNED:
    return sizeof(uint8_t);
  case NT_I16:
    return sizeof(int16_t);
  case NT_I16 | NT_UNSIGNED:
    return sizeof(uint16_t);
  case NT_I32:
    return sizeof(int32_t);
  case NT_I32 | NT_UNSIGNED:
    return sizeof(uint32_t);
  case NT_I64:
    return sizeof(int64_t);
  case NT_I64 | NT_UNSIGNED:
    return sizeof(uint64_t);
  default:
    return 0;
  }
}

void WaveClear(waveHndl wv)
{
  if(wv == nullptr)
  {
    throw IgorException(USING_NULL_REFVAR);
  }

  const auto numBytes = WavePoints(wv) * GetWaveElementSize(WaveType(wv));

  if(numBytes == 0) // nothing to do
  {
    return;
  }

  MemClear(WaveData(wv), numBytes);
}

void ApplyFlags(double flags)
{
  const auto val = lockToIntegerRange<int>(flags);

  int numMatches = 0;

  if(val <= 0)
  {
    throw IgorException(
        UNKNOWN_SET_FLAG,
        fmt::format("zeromq_set: The flag value {} must be positive.\r",
                    flags));
  }

  DEBUG_OUTPUT("ZMQ Library Version {}.{}.{}", ZMQ_VERSION_MAJOR,
               ZMQ_VERSION_MINOR, ZMQ_VERSION_PATCH);

  DEBUG_OUTPUT("git revision {}", GIT_REVISION);

  if(IsBitSet(val, ZeroMQ_SET_FLAGS::DEFAULT))
  {
    GlobalData::Instance().SetDebugFlag(false);
    GlobalData::Instance().SetRecvBusyWaitingFlag(true);
    GlobalData::Instance().SetLoggingFlag(false);
    ToggleIPV6Support(false);
    GlobalData::Instance().SetInterceptorFlag(false);
    GlobalData::Instance().SetClientRecvTimeout(0);
    numMatches++;
  }

  if(IsBitSet(val, ZeroMQ_SET_FLAGS::DEBUG))
  {
    GlobalData::Instance().SetDebugFlag(true);
    numMatches++;
  }

  if(IsBitSet(val, ZeroMQ_SET_FLAGS::IPV6))
  {
    ToggleIPV6Support(true);
    numMatches++;
  }

  if(IsBitSet(val, ZeroMQ_SET_FLAGS::NO_RECV_BUSY_WAITING))
  {
    GlobalData::Instance().SetRecvBusyWaitingFlag(false);
    numMatches++;
  }

  if(IsBitSet(val, ZeroMQ_SET_FLAGS::LOGGING))
  {
    GlobalData::Instance().SetLoggingFlag(true);
    numMatches++;
  }

  if(IsBitSet(val, ZeroMQ_SET_FLAGS::INTERCEPTOR_FUNCTION))
  {
    GlobalData::Instance().SetInterceptorFlag(true);
    numMatches++;
  }

  if(!numMatches)
  {
    throw IgorException(
        UNKNOWN_SET_FLAG,
        fmt::format("zeromq_set: The flag {} is unknown.\r", flags));
  }
}

std::string GetLastEndPoint(void *socket)
{
  std::array<char, MAX_ENDPOINT_LENGTH> buf{};
  size_t bufSize = buf.size();
  const int rc =
      zmq_getsockopt(socket, ZMQ_LAST_ENDPOINT, buf.data(), &bufSize);
  ZEROMQ_ASSERT(rc == 0);

  buf.at(bufSize - 1) = '\0';
  DEBUG_OUTPUT("lastEndPoint={}", buf.data());
  return {buf.data()};
}

void StopThreadsAndCloseConnections()
{
  MessageHandler::Instance().Stop();
  HeartbeatPublisher::Instance().Stop();
  GlobalData::Instance().CloseConnections();
  MessageHandler::Instance().DiscardQueuedMessages();
}

void ToggleIPV6Support(bool enable)
{
  const MessageHandlerPauseGuard pauseGuard(true);

  for(auto st : GetAllSocketTypes())
  {
    GET_SOCKET(clientSocket, st);

    DEBUG_OUTPUT("enable={}", enable);

    const int val = enable;
    auto rc = zmq_setsockopt(clientSocket.get(), ZMQ_IPV6, &val, sizeof(val));
    ZEROMQ_ASSERT(rc == 0);
  }
}

double ConvertStringToDouble(const std::string &str)
{
  char *lastChar = nullptr;
  auto val       = std::strtod(str.c_str(), &lastChar);
  ASSERT(*lastChar == '\0');
  return val;
}

json CallIgorFunctionFromMessage(const std::string &msg,
                                 const std::string &identity)
{
  try
  {
    try
    {
      auto req = RequestInterface::Create(identity, msg);
      return CallIgorFunctionFromReqInterface(req);
    }
    catch(const std::bad_alloc &)
    {
      throw RequestInterfaceException(REQ_OUT_OF_MEMORY);
    }
  }
  catch(const IgorException &e)
  {
    return e;
  }
}

json CallIgorFunctionFromReqInterface(const RequestInterfacePtr &req)
{
  try
  {
    try
    {
      req->CanBeProcessed();
      auto reply = req->Call();

      DEBUG_OUTPUT("Function return value is {:.255s}",
                   reply.dump(DEFAULT_INDENT));

      GlobalData::Instance().AddLogEntry(reply, req->GetCallerIdentity(),
                                         MessageDirection::Outgoing);

      return reply;
    }
    catch(const std::bad_alloc &)
    {
      throw RequestInterfaceException(REQ_OUT_OF_MEMORY);
    }
  }
  catch(const IgorException &e)
  {
    json reply = e;

    if(req->HasValidMessageId())
    {
      reply[MESSAGEID_KEY] = req->GetMessageId();
    }

    const auto history = req->GetHistoryDuringOperation();

    if(!history.empty())
    {
      reply[HISTORY_KEY] = history;
    }

    GlobalData::Instance().AddLogEntry(reply, req->GetCallerIdentity(),
                                       MessageDirection::Outgoing);

    return reply;
  }
}

int ZeroMQClientSend(const std::string &payload)
{
  GET_SOCKET(socket, SocketTypes::Client);
  const auto payloadLength = payload.length();

  DEBUG_OUTPUT("payloadLength={}, socket={}", payloadLength, socket.get());

  // empty
  int rc = zmq_send(socket.get(), nullptr, 0, ZMQ_SNDMORE);
  ZEROMQ_ASSERT(rc == 0);

  // payload
  rc = zmq_send(socket.get(), payload.c_str(), payloadLength, 0);
  ZEROMQ_ASSERT(rc >= 0);

  DEBUG_OUTPUT("rc={}", rc);

  return rc;
}

int ZeroMQServerSend(const std::string &identity, const std::string &payload,
                     bool unreachableIsError)
{
  GET_SOCKET(socket, SocketTypes::Server);
  const auto payloadLength = payload.length();

  DEBUG_OUTPUT("payloadLength={}, socket={}", payloadLength, socket.get());

  // identity
  int rc =
      zmq_send(socket.get(), identity.c_str(), identity.length(), ZMQ_SNDMORE);

  // ZMQ_ROUTER_MANDATORY reports an unknown identity on the first frame
  if(rc < 0 && zmq_errno() == EHOSTUNREACH && !unreachableIsError)
  {
    return -1;
  }

  ZEROMQ_ASSERT(rc >= 0);

  // empty
  rc = zmq_send(socket.get(), nullptr, 0, ZMQ_SNDMORE);
  ZEROMQ_ASSERT(rc == 0);

  // payload
  rc = zmq_send(socket.get(), payload.c_str(), payloadLength, 0);
  ZEROMQ_ASSERT(rc >= 0);

  DEBUG_OUTPUT("rc={}", rc);

  return rc;
}

int ZeroMQPublisherSend(const SendStorageVec &vec)
{
  GET_SOCKET(socket, SocketTypes::Publisher);
  DEBUG_OUTPUT("socket={}", socket.get());

  const auto vecLen = vec.size();
  ASSERT(vecLen >= 2);

  int rc = 0;
  for(size_t i = 0; i < vecLen; i++)
  {
    const int flag = i < (vecLen - 1) ? ZMQ_SNDMORE : 0;

    const auto *const ptr = vec.at(i).GetPtr();
    const auto msgLen     = vec.at(i).GetLength();

    DEBUG_OUTPUT("element[{}]: ptr={}, len={}, flag={}", i, ptr, msgLen, flag);

    rc = zmq_send(socket.get(), ptr, msgLen, flag);
    ZEROMQ_ASSERT(rc >= 0);
  }

  return rc;
}

/// Expect three frames:
/// - identity
/// - empty
/// - payload
int ZeroMQServerReceive(zmq_msg_t *identityMsg, zmq_msg_t *payloadMsg)
{
  GET_SOCKET(socket, SocketTypes::Server);
  auto numBytes = zmq_msg_recv(identityMsg, socket.get(), 0);

  if(numBytes < 0)
  {
    return numBytes;
  }

  // Read the remaining frames of an invalid message, so that the next call
  // starts with the next message, identityMsg keeps the identity
  auto throwInvalidFormat = [&socket](zmq_msg_t *msg)
  {
    while(zmq_msg_more(msg))
    {
      if(zmq_msg_recv(msg, socket.get(), 0) < 0)
      {
        break;
      }
    }

    throw IgorException(INVALID_MESSAGE_FORMAT);
  };

  // zeromq guarantees that either all parts in multi-part messages
  // arrive or none.
  if(!zmq_msg_more(identityMsg))
  {
    throw IgorException(INVALID_MESSAGE_FORMAT);
  }

  numBytes = zmq_msg_recv(payloadMsg, socket.get(), 0);
  if(numBytes != 0 || !zmq_msg_more(payloadMsg))
  {
    throwInvalidFormat(payloadMsg);
  }

  numBytes = zmq_msg_recv(payloadMsg, socket.get(), 0);
  if(numBytes < 0 || zmq_msg_more(payloadMsg))
  {
    throwInvalidFormat(payloadMsg);
  }

  return numBytes;
}

/// Expect two frames:
/// - empty
/// - payload
int ZeroMQClientReceive(zmq_msg_t *payloadMsg)
{
  GET_SOCKET(socket, SocketTypes::Client);
  auto numBytes = zmq_msg_recv(payloadMsg, socket.get(), 0);

  if(numBytes < 0)
  {
    return numBytes;
  }

  // zeromq guarantees that either all parts in multi-part messages
  // arrive or none.
  if(!zmq_msg_more(payloadMsg))
  {
    throw IgorException(INVALID_MESSAGE_FORMAT);
  }

  numBytes = zmq_msg_recv(payloadMsg, socket.get(), 0);

  if(numBytes < 0 || zmq_msg_more(payloadMsg))
  {
    throw IgorException(INVALID_MESSAGE_FORMAT);
  }

  return numBytes;
}

/// Expect at least two frames:
/// - filter
/// - payload
/// - [optionally, more payload]
///
/// Return error code `< 0` or success `0`
int ZeroMQSubscriberReceive(ZeroMQMessageSharedPtrVec &vec,
                            bool allowAdditionalFrames)
{
  vec.resize(0);

  GET_SOCKET(socket, SocketTypes::Subscriber);
  DEBUG_OUTPUT("socket={}", socket.get());

  auto filter = std::make_shared<ZeroMQMessage>();

  auto ret = zmq_msg_recv(filter->get(), socket.get(), 0);
  vec.push_back(filter);

  if(ret < 0)
  {
    return ret;
  }

  if(!zmq_msg_more(filter->get()))
  {
    throw IgorException(INVALID_MESSAGE_FORMAT);
  }

  for(;;)
  {
    auto payload = std::make_shared<ZeroMQMessage>();
    ret          = zmq_msg_recv(payload->get(), socket.get(), 0);
    vec.push_back(payload);

    if(ret < 0)
    {
      return ret;
    }

    auto moreData = zmq_msg_more(payload->get());

    if(moreData)
    {
      if(!allowAdditionalFrames)
      {
        throw IgorException(INVALID_MESSAGE_FORMAT);
      }
    }
    else
    {
      break;
    }
  }

  return 0;
}

std::string SerializeDataFolder(DataFolderHandle dataFolderHandle)
{
  if(!dataFolderHandle)
  {
    return "null";
  }

  DataFolderHandle root   = nullptr;
  DataFolderHandle parent = nullptr;
  auto rc                 = GetRootDataFolder(0, &root);
  ASSERT(rc == 0);

  if(root != dataFolderHandle &&
     GetParentDataFolder(dataFolderHandle, &parent) == NO_PARENT_DATAFOLDER)
  {
    return "free";
  }

  std::array<char, MAXCMDLEN + 1> dataFolderPathOrName{};
  rc = GetDataFolderNameOrPath(dataFolderHandle, 0x1,
                               dataFolderPathOrName.data());
  ASSERT(rc == 0);

  std::string folder(dataFolderPathOrName.data());

  // datafolder handle refers to non existing datafolder
  if(folder.find(":_killed folder_:") != std::string::npos)
  {
    return "null";
  }

  return folder;
}

DataFolderHandle DeSerializeDataFolder(const std::string &path)
{
  if(path.size() >= MAXCMDLEN)
  {
    throw RequestInterfaceException(REQ_INVALID_PARAM_FORMAT);
  }

  DataFolderHandle dataFolderHandle = nullptr;
  auto rc = GetNamedDataFolder(nullptr, path.c_str(), &dataFolderHandle);
  ASSERT(rc == 0);

  return dataFolderHandle;
}

std::string CreateStringFromZMsg(zmq_msg_t *msg)
{
  return std::string(reinterpret_cast<char *>(zmq_msg_data(msg)),
                     zmq_msg_size(msg));
}

void InitHandle(Handle *handle, size_t size)
{
  if(*handle == nullptr)
  {
    *handle = WMNewHandle(size);
    ASSERT(*handle != nullptr);
  }
  else
  {
    const int ret = WMSetHandleSize(*handle, size);
    ASSERT(!ret);
  }
}

void WriteZMsgIntoHandle(Handle *handle, zmq_msg_t *msg)
{
  auto numBytes = zmq_msg_size(msg);

  InitHandle(handle, numBytes);

  if(numBytes > 0)
  {
    auto *rawData = zmq_msg_data(msg);
    ZEROMQ_ASSERT(rawData != nullptr);
    memcpy(**handle, rawData, numBytes);
  }
}

bool IsConvertibleToDouble(const std::string &str)
{
  char *lastChar = nullptr;
  // avoid unused return value warning
  auto val = std::strtod(str.c_str(), &lastChar);
  (void) val;

  return *lastChar == '\0';
}

bool IsWaveType(int igorType)
{
  return IsBitSet(igorType, WAVE_TYPE);
}

bool UsesMultipleReturnValueSyntax(const FunctionInfo &fip)
{
  return fip.returnType == FV_NORETURN_TYPE;
}

int GetNumberOfReturnValues(const FunctionInfo &fip)
{
  // old return value syntax with only one
  if(!UsesMultipleReturnValueSyntax(fip))
  {
    return 1;
  }

  // multiple return value syntax
  // all return values are internally pass-by-ref parameters
  // these are before all other parameters

  // find the first non-pass-by-ref parameter
  const auto *it = std::find_if(
      std::begin(fip.parameterTypes),
      std::next(std::begin(fip.parameterTypes), fip.numRequiredParameters),
      [](int paramType) { return !IsBitSet(paramType, FV_REF_TYPE); });

  // all parameters before `it` are considered return values
  // this does not catch cases where the first input parameter is a
  // pass-by-ref-parameter we currently don't support these cases
  return static_cast<int>(std::distance(std::begin(fip.parameterTypes), it));
}

int GetNumberOfInputParameters(const FunctionInfo &fip, int numReturnValues)
{
  // old return value syntax with only one
  if(!UsesMultipleReturnValueSyntax(fip))
  {
    return static_cast<int>(fip.numRequiredParameters);
  }

  return static_cast<int>(fip.numRequiredParameters) - numReturnValues;
}

int GetFirstInputParameterIndex(const FunctionInfo &fip, int numReturnValues)
{
  return UsesMultipleReturnValueSyntax(fip) ? numReturnValues : 0;
}

// Removes any leading and trailing whitespace and replaces \r with \n
std::string CleanupString(std::string str)
{
  if(str.empty())
  {
    return str;
  }

  constexpr auto whitespace = " \n\r\t";

  const size_t end = str.find_last_not_of(whitespace);
  if(end != std::string::npos)
  {
    str.resize(end + 1);
  }

  const size_t start = str.find_first_not_of(whitespace);
  if(start != std::string::npos)
  {
    str = str.substr(start);
  }

  for(auto &chr : str)
  {
    if(chr == '\r')
    {
      chr = '\n';
    }
  }

  return str;
}

template <>
struct fmt::formatter<OutputMode> : fmt::formatter<std::string>
{
  // parse is inherited from formatter<std::string>.
  template <typename FormatContext>
  auto format(const OutputMode &mode, FormatContext &ctx) const
  {
    std::string name;
    switch(mode)
    {
    case OutputMode::Debug:
      name = "Debug";
      break;
    case OutputMode::Emergency:
      name = "Emergency";
      break;
    case OutputMode::Normal:
      name = "Normal";
      break;
    }

    return formatter<std::string>::format(name, ctx);
  }
};

std::string GetHeader(OutputMode mode, const char *func, int line)
{
  switch(mode)
  {
  case OutputMode::Debug:
  case OutputMode::Emergency: // fallthrough-by-design
    return fmt::format(FMT_STRING("{} {}:L{}: "), mode, func, line);
  case OutputMode::Normal:
    return {};
  }

  ASSERT(0);
}

void vlog(OutputMode mode, const char *func, int line, fmt::string_view format,
          fmt::format_args args) noexcept
{
  try
  {
    if(mode == OutputMode::Debug && !GlobalData::Instance().GetDebugFlag())
    {
      return;
    }

    const auto header = GetHeader(mode, func, line);

    OutputToHistory_TS(header + fmt::vformat(format, args),
                       MapOutputModeToExperimentModification(mode));
  }
  catch(...) // NOLINT(bugprone-empty-catch)
  {
    // the output itself failed, there is nowhere left to report it
  }
}

int CreateDirectory(const std::string &path)
{
#ifdef WINIGOR
  int error = 0;

  // https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createdirectorya
  auto ret = CreateDirectoryA(path.c_str(), nullptr);

  // If the function succeeds, the return value is nonzero
  if(ret)
  {
    return 0;
  }

  error = GetLastError();

  if(error == ERROR_ALREADY_EXISTS)
  {
    return FOLDER_EXISTS_NO_OVERWRITE;
  }
  if(error == ERROR_PATH_NOT_FOUND)
  {
    return CANT_OPEN_FOLDER;
  }

  return INTERNAL_ERROR;
#else
#ifdef MACIGOR
  // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mkdir.2.html
  auto ret = mkdir(path.c_str(), 0777);
  if(!ret)
  {
    return 0;
  }

  if(errno == EEXIST)
  {
    return FOLDER_EXISTS_NO_OVERWRITE;
  }
  if(errno == ENOTDIR)
  {
    return CANT_OPEN_FOLDER;
  }

  return INTERNAL_ERROR;

#else
#error "Unsupported architecture"
#endif
#endif
}

void EnsureDirectoryExists(const std::string &path)
{
  auto ret = CreateDirectory(path);

  if(ret == 0 || ret == FOLDER_EXISTS_NO_OVERWRITE)
  {
    return;
  }

  ASSERT(FullPathPointsToFolder(path.c_str()) == 1);

  throw IgorException(ret);
}

bool IsFreeWave(waveHndl wv)
{
  DataFolderHandle dfH = nullptr;
  const int rc         = GetWavesDataFolder(wv, &dfH);
  ASSERT(rc == 0);

  return dfH == nullptr;
}

void DoBindOrConnect(Handle &handle, SocketTypes st)
{
  const auto point = GetStringFromHandle(handle);
  WMDisposeHandle(handle);
  handle = nullptr;

  const MessageHandlerPauseGuard pauseGuard(st == SocketTypes::Server);

  GET_SOCKET(socket, st);

  int rc = 0;

  switch(st)
  {
  case SocketTypes::Client:
  case SocketTypes::Subscriber:
    rc = zmq_connect(socket.get(), point.c_str());
    break;
  case SocketTypes::Server:
  case SocketTypes::Publisher:
    rc = zmq_bind(socket.get(), point.c_str());
    break;
  }

  ZEROMQ_ASSERT(rc == 0);

  DEBUG_OUTPUT("type={}, point={}, rc={}", st, point, rc);
  GlobalData::Instance().AddToListOfBindsOrConnections(
      GetLastEndPoint(socket.get()), st);

  // zeromq_stop() stops it
  if(st == SocketTypes::Publisher &&
     !HeartbeatPublisher::Instance().IsRunning())
  {
    HeartbeatPublisher::Instance().Start();
  }
}

/// @brief Returns a Igor string Handle from a C++ string
///
/// Returns a Igor string Handle from a C++ string
/// returns nullptr if the Handle could not be created or any other error
/// occured
///
/// @param str C++ string
/// @return Igor string handle with the content of the C++ string
Handle GetHandleFromString(const std::string &str)
{
  Handle handle = nullptr;
  if(!(handle = WMNewHandle(To<BCInt>(str.size()))))
  {
    return nullptr;
  }
  if(PutCStringInHandle(str.c_str(), handle))
  {
    return nullptr;
  }
  return handle;
}

bool IsFreeDataFolder(DataFolderHandle dfr)
{
  DataFolderHandle rootDFR = nullptr;

  int rc = GetRootDataFolder(0, &rootDFR);
  ASSERT(rc == 0);

  if(rootDFR == dfr)
  {
    return false;
  }

  DataFolderHandle parentDFR = nullptr;
  rc                         = GetParentDataFolder(dfr, &parentDFR);
  if(rc == NO_PARENT_DATAFOLDER)
  {
    return true;
  }

  ASSERT(rc == 0);

  return false;
}

size_t GetWaveIndexInMemory(waveHndl w, std::vector<IndexInt> &dims)
{
  if(!w)
  {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS);

  int numDims = 0;
  std::vector<CountInt> dimSizes(MAX_DIMENSIONS + 1, 0);
  const int ret = MDGetWaveDimensions(w, &numDims, dimSizes.data());
  ASSERT(ret == 0);

  dimSizes.resize(To<size_t>(numDims));

  size_t index = 0;
  size_t dim   = 0;
  size_t block = 1;
  for(auto &dimSize : dimSizes)
  {
    auto dimIndex = To<size_t>(dims.at(dim));
    ASSERT(dimIndex < To<size_t>(dimSize));
    index += block * dimIndex;
    block *= To<size_t>(dimSize);
    dim++;
  }

  return index;
}

template <>
void SetWaveElement<std::string>(waveHndl w, std::vector<IndexInt> &dims,
                                 const std::string &value)
{
  if(!w)
  {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS);

  if(WaveType(w) == TEXT_WAVE_TYPE)
  {
    Handle textH  = GetHandleFromString(value);
    const int err = MDSetTextWavePointValue(w, dims.data(), textH);
    WMDisposeHandle(textH);
    if(err)
    {
      throw IgorException(err, "Error writing values to text wave");
    }
  }
  else
  {
    throw IgorException(ERR_INVALID_TYPE, "Wave is not a text wave.");
  }
}

template <>
void SetWaveElement<DataFolderHandle>(waveHndl w, std::vector<IndexInt> &dims,
                                      const DataFolderHandle &value)
{
  if(!w)
  {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS);

  if(WaveType(w) == DATAFOLDER_TYPE)
  {
    auto *address = static_cast<DataFolderHandle *>(WaveData(w));
    address += GetWaveIndexInMemory(w, dims); // NOLINT

    DataFolderHandle target = *address;
    if(target != nullptr && IsFreeDataFolder(target))
    {
      auto err = ReleaseDataFolder(&target);
      ASSERT(err == 0);
    }

    *address = value;
  }
  else
  {
    throw IgorException(ERR_INVALID_TYPE,
                        "Wave is not of data folder reference type.");
  }
}

template <>
void SetWaveElement<waveHndl>(waveHndl w, std::vector<IndexInt> &dims,
                              const waveHndl &value)
{
  if(!w)
  {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS);

  if(WaveType(w) == WAVE_TYPE)
  {
    auto *address = static_cast<waveHndl *>(WaveData(w));
    address += GetWaveIndexInMemory(w, dims); // NOLINT

    waveHndl target = *address;
    if(target != nullptr && IsFreeWave(target))
    {
      auto err = ReleaseWave(&target);
      ASSERT(err == 0);
    }

    *address = value;
  }
  else
  {
    throw IgorException(ERR_INVALID_TYPE,
                        "Wave is not of wave reference type.");
  }
}

template <>
DataFolderHandle GetWaveElement<DataFolderHandle>(waveHndl w,
                                                  std::vector<IndexInt> &dims)
{
  if(!w)
  {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS);

  if(WaveType(w) == WAVE_TYPE)
  {
    auto *address = static_cast<DataFolderHandle *>(WaveData(w));

    address += GetWaveIndexInMemory(w, dims); // NOLINT
    return *address;
  }

  throw IgorException(ERR_INVALID_TYPE,
                      "XOP Bug: Wave is not of data folder reference type.");
}

template <>
waveHndl GetWaveElement<waveHndl>(waveHndl w, std::vector<IndexInt> &dims)
{
  if(!w)
  {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS);

  if(WaveType(w) == WAVE_TYPE)
  {
    auto *address = static_cast<waveHndl *>(WaveData(w));

    address += GetWaveIndexInMemory(w, dims); // NOLINT
    return *address;
  }

  throw IgorException(ERR_INVALID_TYPE,
                      "XOP Bug: Wave is not of wave reference type.");
}

template <>
std::string GetWaveElement<std::string>(waveHndl w, std::vector<IndexInt> &dims)
{
  if(!w)
  {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS);

  if(WaveType(w) == TEXT_WAVE_TYPE)
  {
    Handle textH = nullptr;
    if(!(textH = WMNewHandle(0L)))
    {
      throw IgorException(NOMEM, "Error creating Igor Handle.");
    }
    if(const int err = MDGetTextWavePointValue(w, dims.data(), textH))
    {
      WMDisposeHandle(textH);
      throw IgorException(err, "Error reading value from text wave");
    }
    return GetStringFromHandleWithDispose(textH);
  }

  throw IgorException(ERR_INVALID_TYPE, "XOP Bug: Wave is not a text wave.");
}

void CheckWaveDimension(waveHndl w, const std::vector<CountInt> &expectedDims,
                        const std::string &errorMsg)
{
  std::vector<CountInt> compDims(MAX_DIMENSIONS + 1, 0);
  auto size = To<std::ptrdiff_t>(expectedDims.size() < compDims.size() - 1
                                     ? expectedDims.size()
                                     : compDims.size() - 1);
  std::copy(expectedDims.begin(), expectedDims.begin() + size,
            compDims.begin());

  const std::vector<CountInt> dims = GetWaveDimension(w);
  if(compDims != dims)
  {
    throw IgorException(ERR_INVALID_TYPE, errorMsg);
  }
}

std::vector<CountInt> GetWaveDimension(waveHndl w)
{
  int numDims = 0;
  return GetWaveDimension(w, numDims);
}

std::vector<CountInt> GetWaveDimension(waveHndl w, int &numDims)
{
  if(!w)
  {
    throw IgorException(NOWAV);
  }
  std::vector<CountInt> dims(MAX_DIMENSIONS + 1, 0);
  if(const int err = MDGetWaveDimensions(w, &numDims, dims.data()))
  {
    throw IgorException(err, "Error retrieving wave dimension.");
  }

  return dims;
}

/// @brief Simple wave redimension that keeps type and reshapes accordingly.
void RedimensionWave(waveHndl w, std::vector<CountInt> dims)
{
  if(MDChangeWave2(w, -1, dims.data(), 0))
  {
    throw IgorException(INTERNAL_ERROR, "Error on wave size change.");
  }
}

waveHndl MakeFreeWave(std::vector<CountInt> dims, int type)
{
  dims.resize(MAX_DIMENSIONS + 1, 0);

  waveHndl wv = nullptr;
  auto ret    = MDMakeWave(&wv, "free", reinterpret_cast<DataFolderHandle>(-1),
                           dims.data(), type, -1);

  if(ret || wv == nullptr)
  {
    throw IgorException(GENERAL_BAD_VIBS, "Could not create free wave");
  }

  return wv;
}
