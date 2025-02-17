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
    }
    else
    {
      auto ptr            = GetWaveDataPtr<void>(wv);
      const auto numBytes = WavePoints(wv) * GetWaveElementSize(waveType);
      sendStorage.emplace_back(SendStorage(ptr, numBytes));
    }
  }

  return sendStorage;
}
