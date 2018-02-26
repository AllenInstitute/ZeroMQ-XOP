#include "CallFunctionParameterHandler.h"
#include "ZeroMQ.h"
#include <initializer_list>
#include "SerializeWave.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

bool IsWaveType(int igorType)
{
  return IsBitSet(igorType, WAVE_TYPE) || IsBitSet(igorType, TEXT_WAVE_TYPE);
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
    break;
  case HSTRING_TYPE:
  {
    auto result = GetStringFromHandle(ret->stringHandle);
    WMDisposeHandle(ret->stringHandle);
    ret->stringHandle = nullptr;
    return result;
  }
  break;
  case DATAFOLDER_TYPE:
    return SerializeDataFolder(ret->dataFolderHandle);
    break;
  default:
    if(IsWaveType(igorType))
    {
      return SerializeWave(ret->waveHandle);
    }
    break;
  }
}

std::string GetTypeStringForIgorType(int igorType)
{
  igorType = ClearBit(igorType, FV_REF_TYPE);

  switch(igorType)
  {
  case NT_FP64:
    return "variable";
    break;
  case HSTRING_TYPE:
    return "string";
    break;
  case DATAFOLDER_TYPE:
    return "dfref";
    break;
  default:
    if(IsWaveType(igorType))
    {
      return "wave";
    }
    ASSERT(0);
    break;
  }
}

} // anonymous namespace

CallFunctionParameterHandler::CallFunctionParameterHandler(
    StringVector inputParams, FunctionInfo fip)
    : m_retStorage({}), m_returnType(fip.returnType)
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

  std::memset(&m_values[0], 0, sizeof(m_values));

  m_paramSizesInBytes.resize(fip.numRequiredParameters);
  m_paramTypes.resize(fip.numRequiredParameters);
  std::copy(std::begin(fip.parameterTypes),
            std::begin(fip.parameterTypes) + fip.numRequiredParameters,
            m_paramTypes.begin());

  for(int i = 0; i < fip.numRequiredParameters; i++)
  {
    DebugOutput(fmt::sprintf("%s: Parameter=%d with type %X\r", __func__, i,
                             m_paramTypes[i]));

    const auto igorType = ClearBit(m_paramTypes[i], FV_REF_TYPE);

    switch(igorType)
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
      if(IsWaveType(igorType))
      {
        m_paramSizesInBytes[i] = sizeof(waveHndl);
        break;
      }

      ASSERT(0);
      break;
    }
  }

  const auto firstInputParamIndex =
      GetFirstInputParameterIndex(fip, m_numReturnValues);

  unsigned char *dest = GetParameterValueStorage();
  for(int i = 0; i < fip.numRequiredParameters; i++)
  {
    if(i >= firstInputParamIndex)
    {
      const auto inputParamIndex = i - firstInputParamIndex;

      IgorTypeUnion u;

      const auto type = m_paramTypes[i] & ~FV_REF_TYPE;

      switch(type)
      {
      case NT_FP64:
        u.variable = ConvertStringToDouble(inputParams[inputParamIndex]);
        break;
      case HSTRING_TYPE:
        u.stringHandle = WMNewHandle(inputParams[inputParamIndex].size());
        ASSERT(u.stringHandle != nullptr);
        memcpy(*u.stringHandle, inputParams[i].c_str(),
               inputParams[inputParamIndex].size());
        break;
      case DATAFOLDER_TYPE:
        u.dataFolderHandle =
            DeSerializeDataFolder(inputParams[inputParamIndex].c_str());
        break;
      default:
        ASSERT(0);
        break;
      }

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
    return json();
  }
  else
  {
    return ReadPassByRefParameters(0, INT_MAX);
  }
}

json CallFunctionParameterHandler::GetReturnValues()
{
  if(m_multipleReturnValueSyntax)
  {
    return ReadPassByRefParameters(0, m_numReturnValues);
  }
  else
  {
    json doc;

    // fixme return one array with pairs and not two arrays
    doc["value"] = ExtractFromUnion(&m_retStorage, m_returnType);
    doc["type"]  = GetTypeStringForIgorType(m_returnType);

    return doc;
  }
}

void *CallFunctionParameterHandler::GetReturnValueStorage()
{
  if(m_multipleReturnValueSyntax)
  {
    return nullptr;
  }

  return &m_retStorage;
}

json CallFunctionParameterHandler::ReadPassByRefParameters(int first, int last)
{
  unsigned char *src = GetParameterValueStorage();

  json elems;

  for(int i = 0; i < m_paramTypes.size() && i < last; i++)
  {
    const auto igorType = m_paramTypes[i];

    if(!IsBitSet(igorType, FV_REF_TYPE))
    {
      continue;
    }

    if(i >= first)
    {
      if(IsBitSet(igorType, NT_FP64) || IsBitSet(igorType, HSTRING_TYPE) ||
         IsBitSet(igorType, DATAFOLDER_TYPE) || IsWaveType(igorType))
      {
        json doc;

        doc["value"] = ExtractFromUnion((IgorTypeUnion *) src, m_paramTypes[i]);
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

CallFunctionParameterHandler::~CallFunctionParameterHandler()
{
  // fixme memory leak
  // resurrect old code and fix it
}

unsigned char *CallFunctionParameterHandler::GetParameterValueStorage()
{
  return &m_values[0];
}
