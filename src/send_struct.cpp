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

  const auto numRows = dimensions[0];
  const auto numCols = dimensions[1];

  if(numRows < 2)
  {
    throw IgorException(ERR_INVALID_TYPE);
  }

  if(numCols > 1)
  {
    throw IgorException(ERR_INVALID_TYPE);
  }

  SendStorageVec sendStorage;
  std::string filter, payload;

  for(CountInt i = 0; i < numRows; i += 1)
  {
    std::vector<IndexInt> containerDims(MAX_DIMENSIONS, 0);
    containerDims[0] = i;

    auto wv = GetWaveElement<waveHndl>(containerWaveHandle, containerDims);

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
      std::string elem = GetWaveElement<std::string>(wv, dims);

      sendStorage.emplace_back(SendStorage(elem));

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
      auto ptr            = GetWaveDataPtr<void>(wv);
      const auto numBytes = WavePoints(wv) * GetWaveElementSize(waveType);
      sendStorage.emplace_back(SendStorage(ptr, numBytes));
    }
  }

  GlobalData::Instance().AddLogEntry(filter + ":" + payload + " + [...]",
                                     MessageDirection::Outgoing);

  return sendStorage;
}

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
  resizeDims[0] = numMsg;
  RedimensionWave(containerWaveHandle, resizeDims);

  for(size_t i = 0; i < numMsg; i += 1)
  {
    const auto msg     = vec[i]->get();
    const auto msgSize = zmq_msg_size(msg);

    std::vector<IndexInt> containerDims(MAX_DIMENSIONS, 0);
    containerDims[0] = i;
    auto wv = GetWaveElement<waveHndl>(containerWaveHandle, containerDims);

    int waveType;
    bool isTextWave;

    if(wv == nullptr)
    {
      std::vector<IndexInt> dims(MAX_DIMENSIONS, 0);

      // first two must be single point text waves
      if(i < 2)
      {
        dims[0]  = 1;
        waveType = TEXT_WAVE_TYPE;
      }
      else
      {
        dims[0]  = msgSize;
        waveType = NT_I8 | NT_UNSIGNED;
      }

      isTextWave = (waveType == TEXT_WAVE_TYPE);

      wv      = MakeFreeWave(dims, waveType);
      int ret = HoldWave(wv);
      ASSERT(ret == 0);

      SetWaveElement<waveHndl>(containerWaveHandle, containerDims, wv);
    }
    else
    {
      waveType = WaveType(wv);

      // complain at non-numeric waves
      if(waveType == DATAFOLDER_TYPE || waveType == WAVE_TYPE)
      {
        throw IgorException(ERR_INVALID_TYPE);
      }

      isTextWave = (waveType == TEXT_WAVE_TYPE);

      // first two must be text waves
      if(i < 2 && !isTextWave)
      {
        throw IgorException(ERR_INVALID_TYPE);
      }

      std::vector<IndexInt> dims(MAX_DIMENSIONS, 0);

      if(isTextWave)
      {
        dims[0] = 1;
      }
      else
      {
        dims[0] = msgSize / GetWaveElementSize(waveType);
      }

      RedimensionWave(wv, dims);
    }

    if(isTextWave)
    {
      auto str = CreateStringFromZMsg(msg);
      std::vector<IndexInt> dims(MAX_DIMENSIONS, 0);
      SetWaveElement<std::string>(wv, dims, str);

      DEBUG_OUTPUT("element[{}]: data={}, length={}", i, str, msgSize);
    }
    else
    {
      void *dest      = GetWaveDataPtr<void *>(wv);
      const void *src = zmq_msg_data(msg);

      DEBUG_OUTPUT("element[{}]: ptr={}, length={}", i, src, msgSize);

      memcpy(dest, src, msgSize);
    }
  }

  ASSERT(numMsg >= 2);
  auto filter  = CreateStringFromZMsg(vec[0]->get());
  auto payload = CreateStringFromZMsg(vec[1]->get());

  GlobalData::Instance().AddLogEntry(filter + ":" + payload + " + [...]",
                                     MessageDirection::Incoming);
}
