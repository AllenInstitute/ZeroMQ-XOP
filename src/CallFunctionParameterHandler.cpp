#include "CallFunctionParameterHandler.h"
#include "ZeroMQ.h"
#include "SerializeWave.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

std::string GetTypeStringForIgorType(int igorType)
{
  switch(igorType)
  {
  case NT_FP64:
    return "variable";
  case HSTRING_TYPE:
    return "string";
  case WAVE_TYPE:
    return "wave";
  case DATAFOLDER_TYPE:
    return "dfref";
  default:
    ASSERT(0);
  }
}

json ExtractReturnValueFromUnion(IgorTypeUnion *ret, int returnType)
{
  switch(returnType)
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
  case WAVE_TYPE:
    return SerializeWave(ret->waveHandle);
  case DATAFOLDER_TYPE:
    return SerializeDataFolder(ret->dataFolderHandle);
  default:
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
    : m_returnType(fip.returnType), m_hasPassByRefParams(false)
{
  ASSERT(sizeof(fip.parameterTypes) / sizeof(int) == MAX_NUM_PARAMS);
  ASSERT(fip.totalNumParameters < MAX_NUM_PARAMS);

  m_numInputParams = static_cast<int>(fip.numRequiredParameters);
  ASSERT(m_numInputParams == static_cast<int>(inputParams.size()));

  if(m_numInputParams == 0)
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
    m_hasPassByRefParams |= (m_paramTypes[i] & FV_REF_TYPE) == FV_REF_TYPE;

    const auto type = m_paramTypes[i] & ~FV_REF_TYPE;

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
      ASSERT(0);
    }
  }

  unsigned char *dest = GetParameterValueStorage();
  for(int i = 0; i < fip.numRequiredParameters; i++)
  {
    auto u = ConvertStringToIgorTypeUnion(inputParams[i], m_paramTypes[i]);

    // we write one parameter after another into our array
    // we can not use IgorTypeUnion here as the padding on 32bit
    // (void* is 4, but a double 8) breaks the reading code in CallFunction.
    memcpy(dest, &u, m_paramSizesInBytes[i]);
    dest += m_paramSizesInBytes[i];
  }
}

json CallFunctionParameterHandler::GetPassByRefArray()
{
  unsigned char *src = GetParameterValueStorage();
  IgorTypeUnion u;

  std::vector<std::string> elems;
  elems.reserve(m_paramTypes.size());

  for(size_t i = 0; i < m_paramTypes.size(); i++)
  {
    switch(m_paramTypes[i])
    {
    case NT_FP64 | FV_REF_TYPE:
      memcpy(&u, src, m_paramSizesInBytes[i]);
      elems.push_back(To_stringHighRes(u.variable));
      break;
    case HSTRING_TYPE | FV_REF_TYPE:
      memcpy(&u, src, m_paramSizesInBytes[i]);
      elems.push_back(GetStringFromHandle(u.stringHandle));
      break;
    }

    src += m_paramSizesInBytes[i];
  }

  return json(elems);
}

bool CallFunctionParameterHandler::HasPassByRefParameters()
{
  return m_hasPassByRefParams;
}

unsigned char *CallFunctionParameterHandler::GetParameterValueStorage()
{
  return &m_values[0];
}

json CallFunctionParameterHandler::GetReturnValues()
{
  json doc;

  doc["value"] = ExtractReturnValueFromUnion(&m_retStorage, m_returnType);
  doc["type"]  = GetTypeStringForIgorType(m_returnType);

  return doc;
}

void *CallFunctionParameterHandler::GetReturnValueStorage()
{

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
