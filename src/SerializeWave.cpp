#include "ZeroMQ.h"
#include "SerializeWave.h"

#include <algorithm>
#include <cmath>
#include <utility>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

std::string JSONQuote(const std::string &str)
{
  return json(str).dump();
}

TickCountInt ConvertToUnixEpochUTC(TickCountInt secs)
{
  if(secs == 0)
  {
    // wave is either a free wave or a permanent wave
    // created with Igor Pro earlier than 1.2
    return secs;
  }

  // calculate the offset including local time zone to the unix epoch
  constexpr std::int16_t UNIX_EPOCH_YEAR = 1970;

  std::int16_t year       = UNIX_EPOCH_YEAR;
  std::int16_t month      = 1;
  std::int16_t dayOfMonth = 1;
  double offset           = std::numeric_limits<double>::quiet_NaN();

  auto rc = DateToIgorDateInSeconds(1, &year, &month, &dayOfMonth, &offset);
  ASSERT(rc == 0);

  return secs - static_cast<TickCountInt>(offset);
}

TickCountInt GetModificationDate(waveHndl waveHandle)
{
  return ConvertToUnixEpochUTC(WaveModDate(waveHandle));
}

std::string GetWaveTypeString(int waveType)
{
  std::string result;

  const auto isComplex = IsBitSet(waveType, NT_CMPLX);
  waveType             = ClearBit(waveType, NT_CMPLX);

  const auto isUnsigned = IsBitSet(waveType, NT_UNSIGNED);
  waveType              = ClearBit(waveType, NT_UNSIGNED);

  switch(waveType)
  {
  case NT_FP32:
    result = "NT_FP32";
    break;
  case NT_FP64:
    result = "NT_FP64";
    break;
  case NT_I8:
    result = "NT_I8";
    break;
  case NT_I16:
    result = "NT_I16";
    break;
  case NT_I32:
    result = "NT_I32";
    break;
  case NT_I64:
    result = "NT_I64";
    break;
  case TEXT_WAVE_TYPE:
    result = "TEXT_WAVE_TYPE";
    break;
  case WAVE_TYPE:
    result = "WAVE_TYPE";
    break;
  case DATAFOLDER_TYPE:
    result = "DATAFOLDER_TYPE";
    break;
  default:
    ASSERT(0);
  }

  if(isComplex)
  {
    result += " | NT_CMPLX";
  }

  if(isUnsigned)
  {
    result += " | NT_UNSIGNED";
  }

  return result;
}

template <typename T, int withComma>
struct WriteIntoStream
{
  void operator()(fmt::memory_buffer &buf, T val)
  {
    auto formatSpec = GetFormatString<T, withComma>()();

    fmt::format_to(std::back_inserter(buf), formatSpec, val);
  }
};

template <int withComma>
struct WriteIntoStream<double, withComma>
{
  void operator()(fmt::memory_buffer &buf, double val)
  {
    auto formatSpecNumber = GetFormatString<double, withComma>()();
    auto formatSpecString = GetFormatString<char *, withComma>()();

    if(std::isnan(val) || std::isinf(val))
    {
      fmt::format_to(std::back_inserter(buf), formatSpecString,
                     JSONQuote(std::to_string(val)));
    }
    else
    {
      fmt::format_to(std::back_inserter(buf), formatSpecNumber, val);
    }
  }
};

template <int withComma>
struct WriteIntoStream<float, withComma>
{
  void operator()(fmt::memory_buffer &buf, float val)
  {
    auto formatSpecNumber = GetFormatString<float, withComma>()();
    auto formatSpecString = GetFormatString<char *, withComma>()();

    if(std::isnan(val) || std::isinf(val))
    {
      fmt::format_to(std::back_inserter(buf), formatSpecString,
                     JSONQuote(std::to_string(val)));
    }
    else
    {
      fmt::format_to(std::back_inserter(buf), formatSpecNumber, val);
    }
  }
};

template <int withComma>
struct WriteIntoStream<char *, withComma>
{
  void operator()(fmt::memory_buffer &buf, const char *val)
  {
    auto formatSpec = GetFormatString<char *, withComma>()();

    fmt::format_to(std::back_inserter(buf), formatSpec, JSONQuote(val));
  }
};

template <int withComma>
struct WriteIntoStream<std::string, withComma>
{
  void operator()(fmt::memory_buffer &buf, const std::string &val)
  {
    auto formatSpec = GetFormatString<char *, withComma>()();

    fmt::format_to(std::back_inserter(buf), formatSpec, JSONQuote(val));
  }
};

template <int withComma>
struct WriteIntoStream<waveHndl, withComma>
{
  void operator()(fmt::memory_buffer &buf, waveHndl val)
  {
    auto formatSpec = GetFormatString<waveHndl, withComma>()();

    fmt::format_to(std::back_inserter(buf), formatSpec,
                   SerializeWave(val).dump(DEFAULT_INDENT));
  }
};

template <int withComma>
struct WriteIntoStream<DataFolderHandle, withComma>
{
  void operator()(fmt::memory_buffer &buf, DataFolderHandle val)
  {
    auto formatSpec = GetFormatString<std::string, withComma>()();

    fmt::format_to(std::back_inserter(buf), formatSpec,
                   JSONQuote(SerializeDataFolder(val)));
  }
};

template <typename T>
void OutputArray(fmt::memory_buffer &buf, T *data, CountInt dataLength)
{
  WriteIntoStream<T, 1> streamWriterWithComma;
  WriteIntoStream<T, 0> streamWriter;

  fmt::format_to(std::back_inserter(buf), "[");

  CountInt index = 0;
  for(index = 0; index < dataLength - 1; index++)
  {
    streamWriterWithComma(buf, *std::next(data, index));
  }

  streamWriter(buf, *std::next(data, index));

  fmt::format_to(std::back_inserter(buf), "]");
}

template <typename T>
void ToString(fmt::memory_buffer &buf, waveHndl waveHandle, CountInt offset)
{
  auto dataLength = WavePoints(waveHandle);

  if(dataLength == 0)
  {
    fmt::format_to(std::back_inserter(buf), "[]");
    return;
  }

  T *data = std::next(GetWaveDataPtr<T>(waveHandle), offset);

  OutputArray(buf, data, dataLength);
}

template <>
void ToString<char *>(fmt::memory_buffer &buf, waveHndl waveHandle,
                      CountInt /* offset */)
{
  const auto dataLength = WavePoints(waveHandle);

  if(dataLength == 0)
  {
    fmt::format_to(std::back_inserter(buf), "[]");
    return;
  }

  Handle textHandle = WMNewHandle(0);
  ASSERT(textHandle != nullptr);
  const auto mode = 0;
  auto rc         = GetTextWaveData(waveHandle, mode, &textHandle);
  ASSERT(rc == 0);
  char *data = *textHandle;

  WriteIntoStream<char *, 1> streamWriterWithComma;
  WriteIntoStream<char *, 0> streamWriter;

  fmt::format_to(std::back_inserter(buf), "[");

  CountInt index = 0;
  for(index = 0; index < dataLength - 1; index++)
  {
    streamWriterWithComma(buf, data);
    data = std::next(data, To<std::ptrdiff_t>(strlen(data) + 1));
  }

  streamWriter(buf, data);

  fmt::format_to(std::back_inserter(buf), "]");

  WMDisposeHandle(textHandle);
}

std::string WaveToStringImpl(int waveType, waveHndl waveHandle, CountInt offset)
{
  fmt::memory_buffer buf;

  switch(waveType)
  {
  case NT_FP32:
    ToString<float>(buf, waveHandle, offset);
    break;
  case NT_FP64:
    ToString<double>(buf, waveHandle, offset);
    break;
  case NT_I8:
    ToString<int8_t>(buf, waveHandle, offset);
    break;
  case NT_I16:
    ToString<int16_t>(buf, waveHandle, offset);
    break;
  case NT_I32:
    ToString<int32_t>(buf, waveHandle, offset);
    break;
  case NT_I64:
    ToString<int64_t>(buf, waveHandle, offset);
    break;
  case NT_I8 | NT_UNSIGNED:
    ToString<uint8_t>(buf, waveHandle, offset);
    break;
  case NT_I16 | NT_UNSIGNED:
    ToString<uint16_t>(buf, waveHandle, offset);
    break;
  case NT_I32 | NT_UNSIGNED:
    ToString<uint32_t>(buf, waveHandle, offset);
    break;
  case NT_I64 | NT_UNSIGNED:
    ToString<uint64_t>(buf, waveHandle, offset);
    break;
  case TEXT_WAVE_TYPE:
    ToString<char *>(buf, waveHandle, offset);
    break;
  case WAVE_TYPE:
    ToString<waveHndl>(buf, waveHandle, offset);
    break;
  case DATAFOLDER_TYPE:
    ToString<DataFolderHandle>(buf, waveHandle, offset);
    break;
  default:
    ASSERT(0);
  }

  return to_string(buf);
}

std::string WaveToString(int waveType, waveHndl waveHandle)
{
  const auto isComplex = IsBitSet(waveType, NT_CMPLX);
  waveType             = ClearBit(waveType, NT_CMPLX);

  if(!isComplex)
  {
    return WaveToStringImpl(waveType, waveHandle, 0);
  }

  // WaveToStringImpl returns a JSON quoted string already
  const auto *resultTemplate = R"({{
      "real"     : {},
      "imag"     : {}
      }}
   )";

  return fmt::format(
      resultTemplate, WaveToStringImpl(waveType, waveHandle, 0),
      WaveToStringImpl(waveType, waveHandle, WavePoints(waveHandle)));
}

std::string DimensionSizesToString(std::vector<CountInt> dimensionSizes)
{
  if(dimensionSizes.empty())
  {
    // special case an empty wave
    return "[0]";
  }

  fmt::memory_buffer buf;
  OutputArray(buf, dimensionSizes.data(), To<CountInt>(dimensionSizes.size()));

  return to_string(buf);
}

void AddDataFullScaleIfSet(json &doc, waveHndl waveHandle)
{
  std::vector<double> entries = {std::numeric_limits<double>::quiet_NaN(),
                                 std::numeric_limits<double>::quiet_NaN()};

  // see the documentation to MDGetWaveScaling, the order 1, 0 is correct
  auto rc = MDGetWaveScaling(waveHandle, -1, &entries.at(1), entries.data());
  ASSERT(rc == 0);

  // heuristic;
  // in Igor you would use the first "isValid" entry of the WaveInfo string
  if((entries.at(0) == 0.0 && entries.at(1) == 0.0) ||
     (std::isnan(entries.at(0)) && std::isnan(entries.at(1))))
  {
    return;
  }

  fmt::memory_buffer buf;
  OutputArray(buf, entries.data(), To<CountInt>(entries.size()));

  doc["data"]["fullScale"] = json::parse(to_string(buf));
}

void AddDataUnitIfSet(json &doc, waveHndl waveHandle)
{
  std::array<char, MAX_UNIT_CHARS + 1> unit{};

  auto rc = MDGetWaveUnits(waveHandle, -1, unit.data());
  ASSERT(rc == 0);

  if(strlen(unit.data()) > 0)
  {
    doc["data"]["unit"] = unit.data();
  }
}

void AddDimensionScalingIfSet(json &doc, waveHndl waveHandle,
                              const std::vector<CountInt> &dimSizes)
{
  const auto numDimensions  = dimSizes.size();
  auto differentFromDefault = false;

  std::vector<double> offset(numDimensions);
  std::vector<double> delta(numDimensions);

  for(size_t i = 0; i < numDimensions; i++)
  {
    auto rc = MDGetWaveScaling(waveHandle, static_cast<int>(i), &delta.at(i),
                               &offset.at(i));
    ASSERT(rc == 0);

    differentFromDefault |= (offset.at(i) != 0.0 || delta.at(i) != 1.0);
  }

  if(differentFromDefault)
  {
    fmt::memory_buffer buf;

    OutputArray(buf, offset.data(), To<CountInt>(offset.size()));
    doc["dimension"]["offset"] = json::parse(to_string(buf));

    buf.clear();

    OutputArray(buf, delta.data(), To<CountInt>(delta.size()));
    doc["dimension"]["delta"] = json::parse(to_string(buf));
  }
}

void AddDimensionUnitsIfSet(json &doc, waveHndl waveHandle,
                            const std::vector<CountInt> &dimSizes)
{
  const auto numDimensions  = dimSizes.size();
  auto differentFromDefault = false;

  std::vector<std::string> units(numDimensions);

  for(size_t i = 0; i < numDimensions; i++)
  {
    std::array<char, MAX_UNIT_CHARS + 1> unit{};
    auto rc = MDGetWaveUnits(waveHandle, static_cast<int>(i), unit.data());
    ASSERT(rc == 0);

    if(strlen(unit.data()) > 0)
    {
      units.at(i)          = unit.data();
      differentFromDefault = true;
    }
  }

  if(differentFromDefault)
  {
    fmt::memory_buffer buf;

    OutputArray(buf, units.data(), To<CountInt>(units.size()));
    doc["dimension"]["unit"] = json::parse(to_string(buf));
  }
}

void AddDimensionLabelsFullIfSet(json &doc, waveHndl waveHandle,
                                 const std::vector<CountInt> &dimSizes)
{
  const auto numDimensions  = dimSizes.size();
  auto differentFromDefault = false;

  // label for the full dimension
  std::vector<std::string> labels(numDimensions);

  for(size_t i = 0; i < numDimensions; i++)
  {
    std::array<char, MAX_DIM_LABEL_BYTES + 1> label{};
    auto rc =
        MDGetDimensionLabel(waveHandle, static_cast<int>(i), -1, label.data());
    ASSERT(rc == 0);

    if(strlen(label.data()) > 0)
    {
      labels.at(i)         = label.data();
      differentFromDefault = true;
    }
  }

  if(differentFromDefault)
  {
    fmt::memory_buffer buf;

    OutputArray(buf, labels.data(), To<CountInt>(labels.size()));
    doc["dimension"]["label"]["full"] = json::parse(to_string(buf));
  }
}

void AddDimensionLabelsEachIfSet(json &doc, waveHndl waveHandle,
                                 std::vector<CountInt> dimSizes)
{
  const auto numDimensions  = dimSizes.size();
  auto differentFromDefault = false;
  CountInt labelIndex       = 0;

  // label for the each dimension index
  // serialized here with column-major order
  std::vector<std::string> labels(std::accumulate(
      dimSizes.begin(), dimSizes.end(), static_cast<CountInt>(0)));

  for(size_t i = 0; i < numDimensions; i++)
  {
    for(CountInt j = 0; j < dimSizes.at(i); j++)
    {
      std::array<char, MAX_DIM_LABEL_BYTES + 1> label{};
      auto rc =
          MDGetDimensionLabel(waveHandle, static_cast<int>(i), j, label.data());
      ASSERT(rc == 0);

      if(strlen(label.data()) > 0)
      {
        differentFromDefault  = true;
        labels.at(labelIndex) = label.data();
      }

      labelIndex++;
    }
  }

  if(differentFromDefault)
  {
    fmt::memory_buffer buf;

    OutputArray(buf, labels.data(), To<CountInt>(labels.size()));
    doc["dimension"]["label"]["each"] = json::parse(to_string(buf));
  }
}

void AddWaveNoteIfSet(json &doc, waveHndl waveHandle)
{
  auto *handle = WaveNoteCopy(waveHandle);

  if(handle == nullptr)
  {
    return;
  }

  doc["note"] = json(GetStringFromHandle(handle));
  WMDisposeHandle(handle);
}

} // anonymous namespace

json SerializeWave(waveHndl waveHandle)
{
  if(waveHandle == nullptr)
  {
    return nullptr;
  }

  const auto waveType = WaveType(waveHandle);
  const auto modDate  = GetModificationDate(waveHandle);
  const auto type     = GetWaveTypeString(waveType);
  const auto rawData  = WaveToString(waveType, waveHandle);

  int numDims   = 0;
  auto dimSizes = GetWaveDimension(waveHandle, numDims);
  dimSizes.resize(numDims);
  const auto dimSizesString = DimensionSizesToString(dimSizes);

  DEBUG_OUTPUT(
      "waveType={}, modDate={}, type={}, dimSizes={}, rawData={:.255s}",
      waveType, modDate, type, dimSizesString, rawData);

  json doc;
  doc["type"]                 = type;
  doc["date"]["modification"] = modDate;
  doc["data"]["raw"]          = json::parse(rawData);
  doc["dimension"]["size"]    = json::parse(dimSizesString);

  AddDataUnitIfSet(doc, waveHandle);
  AddDataFullScaleIfSet(doc, waveHandle);
  AddDimensionScalingIfSet(doc, waveHandle, dimSizes);
  AddDimensionUnitsIfSet(doc, waveHandle, dimSizes);
  AddDimensionLabelsEachIfSet(doc, waveHandle, dimSizes);
  AddDimensionLabelsFullIfSet(doc, waveHandle, dimSizes);
  AddWaveNoteIfSet(doc, waveHandle);

  return doc;
}
