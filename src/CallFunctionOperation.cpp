#include "ZeroMQ.h"
#include "CallFunctionOperation.h"
#include "CallFunctionParameterHandler.h"
#include "SerializeWave.h"
#include "HistoryGrabber.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

CallFunctionOperation::CallFunctionOperation(json j)
{
  DebugOutput(fmt::format("{}: size={}\r", __func__, j.size()));

  if(j.size() != 1 && j.size() != 2)
  {
    throw RequestInterfaceException(REQ_INVALID_OPERATION_FORMAT);
  }

  // check first element "name"
  auto it = j.find("name");

  if(it == j.end() || !it.value().is_string())
  {
    throw RequestInterfaceException(REQ_INVALID_OPERATION_FORMAT);
  }

  m_name = it.value().get<std::string>();

  if(m_name.empty())
  {
    throw RequestInterfaceException(REQ_NON_EXISTING_FUNCTION);
  }

  it = j.find("params");

  if(it == j.end())
  {
    if(j.size() != 1) // unknown other objects
    {
      throw RequestInterfaceException(REQ_INVALID_OPERATION_FORMAT);
    }

    // no params
    return;
  }

  if(!it.value().is_array())
  {
    throw RequestInterfaceException(REQ_INVALID_PARAM_FORMAT);
  }

  for(const auto &elem : it.value())
  {
    if(elem.is_string())
    {
      m_params.push_back(elem.get<std::string>());
    }
    else if(elem.is_number())
    {
      m_params.push_back(To_stringHighRes(elem.get<double>()));
    }
    else if(elem.is_boolean())
    {
      m_params.push_back(std::to_string(elem.get<bool>()));
    }
    else
    {
      throw RequestInterfaceException(REQ_INVALID_PARAM_FORMAT);
    }
  }

  DebugOutput(
      fmt::format("{}: CallFunction object could be created.\r", __func__));
}

void CallFunctionOperation::CanBeProcessed() const
{
  DebugOutput(fmt::format("{}: Data={}.\r", __func__, *this));

  FunctionInfo fip;
  auto rc = GetFunctionInfo(m_name.c_str(), &fip);

  // procedures must be compiled
  if(rc == NEED_COMPILE)
  {
    throw RequestInterfaceException(REQ_PROC_NOT_COMPILED);
  }
  // non existing function
  else if(rc == EXPECTED_FUNCTION_NAME)
  {
    throw RequestInterfaceException(REQ_NON_EXISTING_FUNCTION);
  }

  DebugOutput(
      fmt::format("{}: func return value is ={}.\r", __func__, fip.returnType));

  ASSERT(rc == 0);

  const auto numReturnValues = GetNumberOfReturnValues(fip);
  const auto numInputParams  = GetNumberOfInputParameters(fip, numReturnValues);
  const auto multipleReturnValueSyntax = UsesMultipleReturnValueSyntax(fip);

  const auto numParamsSupplied = static_cast<int>(m_params.size());

  DebugOutput(fmt::format(
      "{}: Multiple return value syntax={}, Number of return values={}, Number "
      "of required input parameters = {}, Number of parmeters supplied = {}\r",
      __func__, multipleReturnValueSyntax, numReturnValues, numInputParams,
      numParamsSupplied));

  if(numParamsSupplied < numInputParams)
  {
    throw RequestInterfaceException(REQ_TOO_FEW_FUNCTION_PARAMS);
  }
  else if(numParamsSupplied > numInputParams)
  {
    throw RequestInterfaceException(REQ_TOO_MANY_FUNCTION_PARAMS);
  }

  const auto firstInputParamIndex =
      GetFirstInputParameterIndex(fip, numReturnValues);

  // check passed input parameters
  for(auto i = 0; i < numParamsSupplied; i += 1)
  {
    auto igorType = fip.parameterTypes[firstInputParamIndex + i];

    if(IsBitSet(igorType, NT_FP64))
    {
      if(!IsConvertibleToDouble(m_params[i]))
      {
        throw RequestInterfaceException(REQ_INVALID_PARAM_FORMAT);
      }
    }
  }

  // check the function signature

  // 1: return values
  if(fip.returnType != NT_FP64 && fip.returnType != HSTRING_TYPE &&
     fip.returnType != WAVE_TYPE && fip.returnType != DATAFOLDER_TYPE &&
     fip.returnType != FV_NORETURN_TYPE)
  {
    throw RequestInterfaceException(REQ_UNSUPPORTED_FUNC_RET);
  }

  for(auto i = 0; i < fip.numRequiredParameters; i += 1)
  {
    auto igorType = fip.parameterTypes[i];

    // 2: output parameter (aka multiple return value)
    if(i < firstInputParamIndex)
    {
      if(IsBitSet(igorType, NT_FP64) && !IsBitSet(igorType, NT_CMPLX))
      {
        continue;
      }
      else if(IsBitSet(igorType, HSTRING_TYPE))
      {
        continue;
      }
      else if(IsBitSet(igorType, DATAFOLDER_TYPE))
      {
        continue;
      }
      else if(IsWaveType(igorType))
      {
        continue;
      }

      throw RequestInterfaceException(REQ_UNSUPPORTED_FUNC_RET);
    }
    else
    {
      // 3: input parameter
      if(IsBitSet(igorType, NT_FP64) && !IsBitSet(igorType, NT_CMPLX))
      {
        continue;
      }
      else if(IsBitSet(igorType, HSTRING_TYPE))
      {
        continue;
      }
      else if(IsBitSet(igorType, DATAFOLDER_TYPE))
      {
        continue;
      }

      throw RequestInterfaceException(REQ_UNSUPPORTED_FUNC_SIG);
    }
  }

  DebugOutput(fmt::format("{}: Request Object can be processed.\r", __func__));
}

json CallFunctionOperation::Call()
{
  DebugOutput(fmt::format("{}: Data={}.\r", __func__, *this));

  FunctionInfo fip;
  auto rc = GetFunctionInfo(m_name.c_str(), &fip);
  ASSERT(rc == 0);

  CallFunctionParameterHandler p(m_params, fip);

  HistoryGrabber histGrabber;

  rc = CallFunction(&fip, p.GetParameterValueStorage(),
                    p.GetReturnValueStorage());
  ASSERT(rc == 0);

  auto functionAborted = SpinProcess();

  DebugOutput(fmt::format("{}: Call finished with functionAborted={}\r",
                          __func__, functionAborted));

  if(functionAborted)
  {
    m_historyDuringCall = histGrabber.GetHistoryUntilNow();
    throw RequestInterfaceException(REQ_FUNCTION_ABORTED);
  }

  json doc;
  doc["errorCode"] = {{"value", 0}};
  doc["result"]    = {p.GetReturnValues()};

  auto passByRef = p.GetPassByRefInputArray();

  // we can have optional pass-by-ref parameters like structures
  // and in that case passByRef is empty
  if(!passByRef.empty())
  {
    doc["passByReference"] = {passByRef};
  }

  return doc;
}

std::string CallFunctionOperation::GetHistoryDuringCall() const
{
  return m_historyDuringCall;
}
