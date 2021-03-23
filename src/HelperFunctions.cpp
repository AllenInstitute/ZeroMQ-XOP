#include "ZeroMQ.h"
#include "HelperFunctions.h"
#include "RequestInterface.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

std::string GetStringFromHandle(Handle strHandle)
{
  // Check for special case of null handle.
  if(strHandle == nullptr)
  {
    return "";
  }

  size_t strLen = WMGetHandleSize(strHandle);
  return std::string(*strHandle, strLen);
}

/// @brief Set dimension labels on wave
///
/// @param	h	Wave to set dimension labels on
/// @param	Dimension	Dimension to set labels on
/// @param	dimLabels	vector of labels to set
///
/// dimLabels[k] will be assigned to index k of the wave
///
/// @throws Igor error code
void SetDimensionLabels(waveHndl h, int Dimension,
                        const std::vector<std::string> &dimLabels)
{
  // Check wave exists.
  if(h == nullptr)
  {
    throw IgorException(NULL_WAVE_OP);
  }

  for(size_t k = 0; k < dimLabels.size(); k++)
  {
    if(dimLabels[k].size() == 0)
    {
      // Empty string.  Skip.
      continue;
    }

    if(int RetVal = MDSetDimensionLabel(h, Dimension, k, dimLabels[k].c_str()))
    {
      throw IgorException(RetVal);
    }
  }
}

void DebugOutput(std::string str)
{
  if(GlobalData::Instance().GetDebugFlag())
  {
    XOPNotice_ts("DEBUG: " + str);
  }
}

bool IsBitSet(int val, int bit)
{
  return (val & bit) == bit;
}

int ClearBit(int val, int bit)
{
  return val & ~bit;
}

int SetBit(int val, int bit)
{
  return val | bit;
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
        fmt::format("zeromq_set: The flag value {} must positive.\r", flags));
  }

  DebugOutput(fmt::format("{}: ZMQ Library Version {}.{}.{}\r", __func__,
                          ZMQ_VERSION_MAJOR, ZMQ_VERSION_MINOR,
                          ZMQ_VERSION_PATCH));

  DebugOutput(fmt::format("{}: git revision {}\r", __func__, GIT_REVISION));

  if((val & ZeroMQ_SET_FLAGS::DEFAULT) == ZeroMQ_SET_FLAGS::DEFAULT)
  {
    GlobalData::Instance().SetDebugFlag(false);
    GlobalData::Instance().SetRecvBusyWaitingFlag(true);
    ToggleIPV6Support(false);
    numMatches++;
  }

  if((val & ZeroMQ_SET_FLAGS::DEBUG) == ZeroMQ_SET_FLAGS::DEBUG)
  {
    GlobalData::Instance().SetDebugFlag(true);
    numMatches++;
  }

  if((val & ZeroMQ_SET_FLAGS::IPV6) == ZeroMQ_SET_FLAGS::IPV6)
  {
    ToggleIPV6Support(true);
    numMatches++;
  }

  if((val & ZeroMQ_SET_FLAGS::NO_RECV_BUSY_WAITING) ==
     ZeroMQ_SET_FLAGS::NO_RECV_BUSY_WAITING)
  {
    GlobalData::Instance().SetRecvBusyWaitingFlag(false);
    numMatches++;
  }

  if(!numMatches)
  {
    throw IgorException(
        UNKNOWN_SET_FLAG,
        fmt::format("zeromq_set: The flag {} is unknown.\r", flags));
  }
}

std::string GetLastEndPoint(void *s)
{
  char buf[256];
  size_t bufSize = sizeof(buf);
  int rc         = zmq_getsockopt(s, ZMQ_LAST_ENDPOINT, buf, &bufSize);
  ZEROMQ_ASSERT(rc == 0);

  DebugOutput(fmt::format("{}: lastEndPoint={}\r", __func__, buf));
  return std::string(buf);
}

void ToggleIPV6Support(bool enable)
{
  GET_CLIENT_SOCKET(clientSocket);

  DebugOutput(fmt::format("{}: enable={}\r", __func__, enable));

  const int val = enable;
  auto rc = zmq_setsockopt(clientSocket.get(), ZMQ_IPV6, &val, sizeof(val));
  ZEROMQ_ASSERT(rc == 0);

  GET_SERVER_SOCKET(serverSocket);

  rc = zmq_setsockopt(serverSocket.get(), ZMQ_IPV6, &val, sizeof(val));
  ZEROMQ_ASSERT(rc == 0);
}

double ConvertStringToDouble(std::string str)
{
  char *lastChar;
  auto val = std::strtod(str.c_str(), &lastChar);
  ASSERT(*lastChar == '\0');
  return val;
}

std::string CallIgorFunctionFromMessage(std::string msg)
{
  try
  {
    try
    {
      RequestInterface req(msg);

      req.CanBeProcessed();
      auto reply = req.Call();

      DebugOutput(fmt::format("{}: Function return value is {:255s}\r",
                              __func__, reply.dump(4)));

      return reply.dump(4);
    }
    catch(const std::bad_alloc &)
    {
      throw RequestInterfaceException(REQ_OUT_OF_MEMORY);
    }
  }
  catch(const IgorException &e)
  {
    const json reply = e;
    return reply.dump(4);
  }
}

int ZeroMQClientSend(std::string payload)
{
  GET_CLIENT_SOCKET(socket);
  int rc;
  const auto payloadLength = payload.length();

  DebugOutput(fmt::format("{}: payloadLength={}, socket={}\r", __func__,
                          payloadLength, socket.get()));

  // empty
  rc = zmq_send(socket.get(), nullptr, 0, ZMQ_SNDMORE);
  ZEROMQ_ASSERT(rc == 0);

  // payload
  rc = zmq_send(socket.get(), payload.c_str(), payloadLength, 0);
  ZEROMQ_ASSERT(rc > 0);

  DebugOutput(fmt::format("{}: rc={}\r", __func__, rc));

  return rc;
}

int ZeroMQServerSend(std::string identity, std::string payload)
{
  GET_SERVER_SOCKET(socket);
  int rc;
  const auto payloadLength = payload.length();

  DebugOutput(fmt::format("{}: payloadLength={}, socket={}\r", __func__,
                          payloadLength, socket.get()));

  // identity
  rc = zmq_send(socket.get(), identity.c_str(), identity.length(), ZMQ_SNDMORE);
  ZEROMQ_ASSERT(rc > 0);

  // empty
  rc = zmq_send(socket.get(), nullptr, 0, ZMQ_SNDMORE);
  ZEROMQ_ASSERT(rc == 0);

  // payload
  rc = zmq_send(socket.get(), payload.c_str(), payloadLength, 0);
  ZEROMQ_ASSERT(rc > 0);

  DebugOutput(fmt::format("{}: rc={}\r", __func__, rc));

  return rc;
}

/// Expect three frames:
/// - identity
/// - empty
/// - payload
int ZeroMQServerReceive(zmq_msg_t *identityMsg, zmq_msg_t *payloadMsg)
{
  GET_SERVER_SOCKET(socket);
  auto numBytes = zmq_msg_recv(identityMsg, socket.get(), 0);

  if(numBytes < 0)
  {
    return numBytes;
  }

  // zeromq guarantees that either all parts in multi-part messages
  // arrive or none.
  if(!zmq_msg_more(identityMsg))
  {
    throw IgorException(INVALID_MESSAGE_FORMAT);
  }

  numBytes = zmq_msg_recv(payloadMsg, socket.get(), 0);
  if(numBytes != 0 || !zmq_msg_more(payloadMsg))
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

/// Expect two frames:
/// - empty
/// - payload
int ZeroMQClientReceive(zmq_msg_t *payloadMsg)
{
  GET_CLIENT_SOCKET(socket);
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

std::string SerializeDataFolder(DataFolderHandle dataFolderHandle)
{
  if(!dataFolderHandle)
  {
    return "null";
  }

  DataFolderHandle root, parent;
  auto rc = GetRootDataFolder(0, &root);
  ASSERT(rc == 0);

  if(root != dataFolderHandle &&
     GetParentDataFolder(dataFolderHandle, &parent) == NO_PARENT_DATAFOLDER)
  {
    return "free";
  }

  char dataFolderPathOrName[MAXCMDLEN + 1];
  rc = GetDataFolderNameOrPath(dataFolderHandle, 0x1, dataFolderPathOrName);
  ASSERT(rc == 0);

  std::string folder(dataFolderPathOrName);

  // datafolder handle refers to non existing datafolder
  if(folder.find(":_killed folder_:") != std::string::npos)
  {
    return "null";
  }

  return folder;
}

DataFolderHandle DeSerializeDataFolder(std::string path)
{
  if(path.size() >= MAXCMDLEN)
  {
    throw RequestInterfaceException(REQ_INVALID_PARAM_FORMAT);
  }

  DataFolderHandle dataFolderHandle;
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
    int ret = WMSetHandleSize(*handle, size);
    ASSERT(!ret);
  }
}

void WriteZMsgIntoHandle(Handle *handle, zmq_msg_t *msg)
{
  auto numBytes = zmq_msg_size(msg);

  InitHandle(handle, numBytes);

  if(numBytes > 0)
  {
    auto rawData = zmq_msg_data(msg);
    ZEROMQ_ASSERT(rawData != nullptr);
    memcpy(**handle, rawData, numBytes);
  }
}
