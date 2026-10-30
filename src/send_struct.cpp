#include "send_struct.h"

#include "ZeroMQ.h"

SendStorageVec GatherPubData(waveHndl containerWaveHandle)
{
  if(containerWaveHandle == nullptr)
  {
    throw IgorException(NOWAV);
  }

  if(WaveType(containerWaveHandle) != WAVE_TYPE)
  {
    throw IgorException(ERR_INVALID_TYPE);
  }

  const auto dimensions = GetWaveDimension(containerWaveHandle);

  const auto numRows = dimensions.at(0);
  const auto numCols = dimensions.at(1);

  if(numRows < 2)
  {
    throw IgorException(ERR_INVALID_TYPE);
  }

  if(numCols > 1)
  {
    throw IgorException(ERR_INVALID_TYPE);
  }

  SendStorageVec sendStorage;
  std::string filter;
  std::string payload;

  for(CountInt i = 0; i < numRows; i += 1)
  {
    std::vector<IndexInt> containerDims(MAX_DIMENSIONS, 0);
    containerDims.at(0) = i;

    auto *wv = GetWaveElement<waveHndl>(containerWaveHandle, containerDims);

    if(wv == nullptr)
    {
      throw IgorException(NOWAV);
    }

    const auto waveType = WaveType(wv);

    // complain at non-numeric waves
    if(waveType == DATAFOLDER_TYPE || waveType == WAVE_TYPE)
    {
      throw IgorException(ERR_INVALID_TYPE);
    }

    const bool isTextWave = (waveType == TEXT_WAVE_TYPE);

    // first two must be text waves
    if(i < 2 && !isTextWave)
    {
      throw IgorException(ERR_INVALID_TYPE);
    }

    if(isTextWave)
    {
      // require for text waves only a single entry
      CheckWaveDimension(wv, {1}, "Text waves need to be single point only");

      std::vector<IndexInt> dims(MAX_DIMENSIONS, 0);
      const std::string elem = GetWaveElement<std::string>(wv, dims);

      sendStorage.emplace_back(elem);

      if(i == 0)
      {
        filter = elem;
      }
      else if(i == 1)
      {
        payload = elem;
      }
    }
    else
    {
      auto *ptr           = GetWaveDataPtr<void>(wv);
      const auto numBytes = WavePoints(wv) * GetWaveElementSize(waveType);
      sendStorage.emplace_back(ptr, numBytes);
    }
  }

  GlobalData::Instance().AddLogEntry(filter + ":" + payload + " + [...]",
                                     MessageDirection::Outgoing);

  return sendStorage;
}

namespace
{

/// Return the wave for the message at index of the container wave, creating
/// or resizing it for the message size
waveHndl GetMessageWave(waveHndl containerWaveHandle, size_t index,
                        size_t msgSize)
{
  std::vector<IndexInt> containerDims(MAX_DIMENSIONS, 0);
  containerDims.at(0) = To<IndexInt>(index);
  auto *wv = GetWaveElement<waveHndl>(containerWaveHandle, containerDims);

  std::vector<IndexInt> dims(MAX_DIMENSIONS, 0);

  if(wv == nullptr)
  {
    int waveType = 0;

    // first two must be single point text waves
    if(index < 2)
    {
      dims.at(0) = 1;
      waveType   = TEXT_WAVE_TYPE;
    }
    else
    {
      dims.at(0) = To<IndexInt>(msgSize);
      waveType   = NT_I8 | NT_UNSIGNED;
    }

    wv            = MakeFreeWave(dims, waveType);
    const int ret = HoldWave(wv);
    ASSERT(ret == 0);

    SetWaveElement<waveHndl>(containerWaveHandle, containerDims, wv);

    return wv;
  }

  const int waveType = WaveType(wv);

  // complain at non-numeric waves
  if(waveType == DATAFOLDER_TYPE || waveType == WAVE_TYPE)
  {
    throw IgorException(ERR_INVALID_TYPE);
  }

  const bool isTextWave = (waveType == TEXT_WAVE_TYPE);

  // first two must be text waves
  if(index < 2 && !isTextWave)
  {
    throw IgorException(ERR_INVALID_TYPE);
  }

  if(isTextWave)
  {
    dims.at(0) = 1;
  }
  else
  {
    dims.at(0) = To<IndexInt>(msgSize / GetWaveElementSize(waveType));
  }

  RedimensionWave(wv, dims);

  return wv;
}

void WriteMessageIntoWave(waveHndl wv, zmq_msg_t *msg, size_t index)
{
  const auto msgSize = zmq_msg_size(msg);

  if(WaveType(wv) == TEXT_WAVE_TYPE)
  {
    auto str = CreateStringFromZMsg(msg);
    std::vector<IndexInt> dims(MAX_DIMENSIONS, 0);
    SetWaveElement<std::string>(wv, dims, str);

    DEBUG_OUTPUT("element[{}]: data={}, length={}", index, str, msgSize);
    return;
  }

  void *dest      = GetWaveDataPtr<void>(wv);
  const void *src = zmq_msg_data(msg);

  DEBUG_OUTPUT("element[{}]: ptr={}, length={}", index, src, msgSize);

  memcpy(dest, src, msgSize);
}

} // anonymous namespace

void ConvertSubData(const ZeroMQMessageSharedPtrVec &vec,
                    waveHndl containerWaveHandle)
{
  const auto numMsg = vec.size();
  DEBUG_OUTPUT("number of messages={}", numMsg);

  if(containerWaveHandle == nullptr)
  {
    throw IgorException(NOWAV);
  }

  if(WaveType(containerWaveHandle) != WAVE_TYPE)
  {
    throw IgorException(ERR_INVALID_TYPE);
  }

  std::vector<IndexInt> resizeDims(MAX_DIMENSIONS, 0);
  resizeDims.at(0) = To<IndexInt>(numMsg);
  RedimensionWave(containerWaveHandle, resizeDims);

  for(size_t i = 0; i < numMsg; i += 1)
  {
    auto *const msg = vec.at(i)->get();
    auto *wv        = GetMessageWave(containerWaveHandle, i, zmq_msg_size(msg));

    WriteMessageIntoWave(wv, msg, i);
  }

  ASSERT(numMsg >= 2);
  auto filter  = CreateStringFromZMsg(vec.at(0)->get());
  auto payload = CreateStringFromZMsg(vec.at(1)->get());

  GlobalData::Instance().AddLogEntry(filter + ":" + payload + " + [...]",
                                     MessageDirection::Incoming);
}
