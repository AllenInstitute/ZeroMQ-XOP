#include "CallFunctionParameterHandler.h"
#include "ZeroMQ.h"
#include "SerializeWave.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

std::string GetTypeStringForIgorType(int igorType)
{
  igorType = ClearBit(igorType, FV_REF_TYPE);

  switch(igorType)
  {
  case NT_FP64:
    return "variable";
  case HSTRING_TYPE:
    return "string";
  case DATAFOLDER_TYPE:
    return "dfref";
  default:
    if(IsWaveType(igorType))
    {
      return "wave";
    }
    ASSERT(0);
  }
}
json ExtractFromUnion(IgorTypeUnion *ret, int igorType)
{
  igorType = ClearBit(igorType, FV_REF_TYPE);

  switch(igorType)
  {
  case NT_FP64:
    if(isfinite(ret->variable))
    {
      return json::parse(To_stringHighRes(ret->variable));
    }
    else
    {
      return To_stringHighRes(ret->variable);
    }
  case HSTRING_TYPE:
  {
    auto result = GetStringFromHandle(ret->stringHandle);
    WMDisposeHandle(ret->stringHandle);
    ret->stringHandle = nullptr;
    return std::move(result);
  }
  case DATAFOLDER_TYPE:
  {
    auto result = SerializeDataFolder(ret->dataFolderHandle);
    if(ret->dataFolderHandle != nullptr)
    {
      ReleaseDataFolder(&ret->dataFolderHandle);
      ret->dataFolderHandle = nullptr;
    }
    return std::move(result);
  }
  default:
    if(IsWaveType(igorType))
    {
      auto result = SerializeWave(ret->waveHandle);

      if(ret->waveHandle != nullptr)
      {
        ReleaseWave(&ret->waveHandle);
        ret->waveHandle = nullptr;
      }

      return std::move(result);
    }
    ASSERT(0);
  }
}

IgorTypeUnion ConvertStringToIgorTypeUnion(std::string param, int igorType)
{
  igorType = ClearBit(igorType, FV_REF_TYPE);

  IgorTypeUnion u = {};

  switch(igorType)
  {
  case NT_FP64:
    u.variable = ConvertStringToDouble(param);
    break;
  case HSTRING_TYPE:
    u.stringHandle = WMNewHandle(param.size());
    ASSERT(u.stringHandle != nullptr);
    memcpy(*u.stringHandle, param.c_str(), param.size());
    break;
  case DATAFOLDER_TYPE:
    u.dataFolderHandle = DeSerializeDataFolder(param.c_str());
    break;
  default:
    ASSERT(0);
  }

  return u;
}

} // anonymous namespace

CallFunctionParameterHandler::CallFunctionParameterHandler(
    StringVector inputParams, FunctionInfo fip)
    : m_returnType(fip.returnType)
{
  ASSERT(sizeof(fip.parameterTypes) / sizeof(int) == MAX_NUM_PARAMS);
  ASSERT(fip.totalNumParameters < MAX_NUM_PARAMS);

  m_multipleReturnValueSyntax = UsesMultipleReturnValueSyntax(fip);
  m_numReturnValues           = GetNumberOfReturnValues(fip);
  m_numInputParams = GetNumberOfInputParameters(fip, m_numReturnValues);

  ASSERT(m_numInputParams == static_cast<int>(inputParams.size()));

  if(m_numInputParams == 0 && !m_multipleReturnValueSyntax)
  {
    return;
  }

  m_paramSizesInBytes.resize(fip.numRequiredParameters);
  m_paramTypes.resize(fip.numRequiredParameters);
  std::copy(std::begin(fip.parameterTypes),
            std::begin(fip.parameterTypes) + fip.numRequiredParameters,
            m_paramTypes.begin());

  for(int i = 0; i < fip.numRequiredParameters; i++)
  {
    DebugOutput(fmt::format("{}: Parameter={} with type {:X}\r", __func__, i,
                            m_paramTypes[i]));

    const auto type = ClearBit(m_paramTypes[i], FV_REF_TYPE);

    switch(type)
    {
    case NT_FP64:
      m_paramSizesInBytes[i] = sizeof(double);
      break;
    case HSTRING_TYPE:
      m_paramSizesInBytes[i] = sizeof(Handle);
      break;
    case DATAFOLDER_TYPE:
      m_paramSizesInBytes[i] = sizeof(DataFolderHandle);
      break;
    default:
      if(IsWaveType(type))
      {
        m_paramSizesInBytes[i] = sizeof(waveHndl);
        break;
      }
      ASSERT(0);
    }
  }

  const auto firstInputParamIndex =
      GetFirstInputParameterIndex(fip, m_numReturnValues);
  unsigned char *dest = GetParameterValueStorage();
  for(int i = 0; i < fip.numRequiredParameters; i++)
  {
    if(i >= firstInputParamIndex)
    {
      const auto u = ConvertStringToIgorTypeUnion(
          inputParams[i - firstInputParamIndex], m_paramTypes[i]);

      // we write one parameter after another into our array
      // we can not use IgorTypeUnion here as the padding on 32bit
      // (void* is 4, but a double 8) breaks the reading code in CallFunction.
      memcpy(dest, &u, m_paramSizesInBytes[i]);
    }

    dest += m_paramSizesInBytes[i];
  }
}

json CallFunctionParameterHandler::GetPassByRefInputArray()
{
  if(m_multipleReturnValueSyntax)
  {
    return ReadPassByRefParameters(m_numReturnValues, INT_MAX);
  }

  return ReadPassByRefParameters(0, INT_MAX);
}

json CallFunctionParameterHandler::GetReturnValues()
{
  if(m_multipleReturnValueSyntax)
  {
    return ReadPassByRefParameters(0, m_numReturnValues);
  }

  json doc;

  doc["value"] = ExtractFromUnion(&m_retStorage, m_returnType);
  doc["type"]  = GetTypeStringForIgorType(m_returnType);

  return doc;
}

void *CallFunctionParameterHandler::GetReturnValueStorage()
{
  if(m_multipleReturnValueSyntax)
  {
    return nullptr;
  }

  return &m_retStorage;
}

CallFunctionParameterHandler::~CallFunctionParameterHandler()
{
  unsigned char *src = GetParameterValueStorage();
  IgorTypeUnion u;

  for(size_t i = 0; i < m_paramTypes.size(); i++)
  {
    switch(m_paramTypes[i])
    {
    case HSTRING_TYPE | FV_REF_TYPE:
      memcpy(&u, src, m_paramSizesInBytes[i]);
      if(u.stringHandle != nullptr)
      {
        WMDisposeHandle(u.stringHandle);
      }
      break;
    }

    src += m_paramSizesInBytes[i];
  }
}

json CallFunctionParameterHandler::ReadPassByRefParameters(int first, int last)
{
  unsigned char *src = GetParameterValueStorage();

  json elems = {};

  for(int i = 0; i < static_cast<int>(m_paramTypes.size()) && i < last; i++)
  {
    const auto igorType = m_paramTypes[i];

    if(i >= first && IsBitSet(igorType, FV_REF_TYPE))
    {
      if(IsBitSet(igorType, NT_FP64) || IsBitSet(igorType, HSTRING_TYPE) ||
         IsBitSet(igorType, DATAFOLDER_TYPE) || IsWaveType(igorType))
      {
        json doc;

        doc["value"] = ExtractFromUnion(reinterpret_cast<IgorTypeUnion *>(src),
                                        m_paramTypes[i]);
        doc["type"]  = GetTypeStringForIgorType(m_paramTypes[i]);

        elems.push_back(doc);
      }
      else
      {
        ASSERT(0);
      }
    }

    src += m_paramSizesInBytes[i];
  }

  return elems;
}

unsigned char *CallFunctionParameterHandler::GetParameterValueStorage()
{
  return &m_values[0];
}
