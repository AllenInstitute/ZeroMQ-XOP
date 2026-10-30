#include "ZeroMQ.h"
#include "CallFunctionOperation.h"
#include "CallFunctionParameterHandler.h"
#include "HistoryGrabber.h"
#include "SerializeWave.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

bool IsSupportedReturnType(int returnType)
{
  return returnType == NT_FP64 || returnType == HSTRING_TYPE ||
         returnType == WAVE_TYPE || returnType == DATAFOLDER_TYPE ||
         returnType == FV_NORETURN_TYPE;
}

bool IsSupportedInputParameterType(int igorType)
{
  return (IsBitSet(igorType, NT_FP64) && !IsBitSet(igorType, NT_CMPLX)) ||
         IsBitSet(igorType, HSTRING_TYPE) ||
         IsBitSet(igorType, DATAFOLDER_TYPE);
}

/// Output parameters of the multiple return value syntax can also be waves
bool IsSupportedOutputParameterType(int igorType)
{
  return IsSupportedInputParameterType(igorType) || IsWaveType(igorType);
}

} // anonymous namespace

CallFunctionOperation::CallFunctionOperation(json doc) : m_json(std::move(doc))
{
  DEBUG_OUTPUT("size={}", m_json.size());

  if(m_json.size() != 1 && m_json.size() != 2)
  {
    throw RequestInterfaceException(REQ_INVALID_OPERATION_FORMAT);
  }

  // check first element "name"
  auto it = m_json.find("name");

  if(it == m_json.end() || !it.value().is_string())
  {
    throw RequestInterfaceException(REQ_INVALID_OPERATION_FORMAT);
  }

  m_name = it.value().get<std::string>();

  if(m_name.empty())
  {
    throw RequestInterfaceException(REQ_NON_EXISTING_FUNCTION);
  }

  it = m_json.find("params");

  if(it == m_json.end())
  {
    if(m_json.size() != 1) // unknown other objects
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

  DEBUG_OUTPUT("CallFunction object could be created: {}", *this);
}

void CallFunctionOperation::CanBeProcessed() const
{
  DEBUG_OUTPUT("Data={}", *this);

  FunctionInfo fip;
  auto rc = GetFunctionInfo(m_name.c_str(), &fip);

  // procedures must be compiled
  if(rc == NEED_COMPILE)
  {
    throw RequestInterfaceException(REQ_PROC_NOT_COMPILED);
  }
  // non existing function
  if(rc == EXPECTED_FUNCTION_NAME)
  {
    throw RequestInterfaceException(REQ_NON_EXISTING_FUNCTION);
  }

  DEBUG_OUTPUT("func return value is ={}.", fip.returnType);

  ASSERT(rc == 0);

  const auto numReturnValues = GetNumberOfReturnValues(fip);
  const auto numInputParams  = GetNumberOfInputParameters(fip, numReturnValues);
  const auto multipleReturnValueSyntax = UsesMultipleReturnValueSyntax(fip);

  const auto numParamsSupplied = static_cast<int>(m_params.size());

  DEBUG_OUTPUT(
      "Multiple return value syntax={}, Number of return values={}, Number "
      "of required input parameters = {}, Number of parmeters supplied = {}",
      multipleReturnValueSyntax, numReturnValues, numInputParams,
      numParamsSupplied);

  if(numParamsSupplied < numInputParams)
  {
    throw RequestInterfaceException(REQ_TOO_FEW_FUNCTION_PARAMS);
  }
  if(numParamsSupplied > numInputParams)
  {
    throw RequestInterfaceException(REQ_TOO_MANY_FUNCTION_PARAMS);
  }

  const auto firstInputParamIndex =
      GetFirstInputParameterIndex(fip, numReturnValues);

  const std::vector<int> paramTypes(std::begin(fip.parameterTypes),
                                    std::end(fip.parameterTypes));

  // check passed input parameters
  for(auto i = 0; i < numParamsSupplied; i += 1)
  {
    const auto igorType = paramTypes.at(firstInputParamIndex + i);

    if(IsBitSet(igorType, NT_FP64) && !IsConvertibleToDouble(m_params.at(i)))
    {
      throw RequestInterfaceException(REQ_INVALID_PARAM_FORMAT);
    }
  }

  // check the function signature

  // 1: return values
  if(!IsSupportedReturnType(fip.returnType))
  {
    throw RequestInterfaceException(REQ_UNSUPPORTED_FUNC_RET);
  }

  for(auto i = 0; i < fip.numRequiredParameters; i += 1)
  {
    const auto igorType = paramTypes.at(i);

    // 2: output parameter (aka multiple return value)
    if(i < firstInputParamIndex)
    {
      if(!IsSupportedOutputParameterType(igorType))
      {
        throw RequestInterfaceException(REQ_UNSUPPORTED_FUNC_RET);
      }

      continue;
    }

    // 3: input parameter
    if(!IsSupportedInputParameterType(igorType))
    {
      throw RequestInterfaceException(REQ_UNSUPPORTED_FUNC_SIG);
    }
  }

  DEBUG_OUTPUT("Request Object can be processed: {}", *this);
}

void CallFunctionOperation::CallInterceptor(const std::string &identity,
                                            InterceptorMode mode) const
{
  if(!GlobalData::Instance().GetInterceptorFlag())
  {
    return;
  }

  auto funcName = GlobalData::Instance().GetInterceptorFunctionName();
  ASSERT(!funcName.empty());

  DEBUG_OUTPUT("InterceptorName={}", funcName);

  FunctionInfo fip;
  auto rc = GetFunctionInfo(funcName.c_str(), &fip);
  if(rc != 0)
  {
    throw IgorException(NO_INTERCEPTOR_FUNC);
  }

  // we don't check the signature again, CallFunctionParameterHandler does that
  // although with asserts only
  auto mode_str = std::to_string(
      static_cast<std::underlying_type_t<InterceptorMode>>(mode));

  const std::vector<std::string> params{m_json.dump(), identity, mode_str};

  DEBUG_OUTPUT("params={}", params);

  CallFunctionParameterHandler paramHandler(params, fip);

  rc = CallFunction(&fip, paramHandler.GetParameterValueStorage(),
                    paramHandler.GetReturnValueStorage());
  ASSERT(rc == 0);

  auto functionAborted = SpinProcess();

  DEBUG_OUTPUT("Interceptor call finished with functionAborted={}",
               functionAborted);

  if(functionAborted)
  {
    throw RequestInterfaceException(REQ_INTERCEPT_FUNC_ABORTED);
  }
}

json CallFunctionOperation::Call()
{
  DEBUG_OUTPUT("Data={}", *this);

  FunctionInfo fip;
  auto rc = GetFunctionInfo(m_name.c_str(), &fip);
  ASSERT(rc == 0);

  CallFunctionParameterHandler paramHandler(m_params, fip);

  const HistoryGrabber histGrabber;

  rc = CallFunction(&fip, paramHandler.GetParameterValueStorage(),
                    paramHandler.GetReturnValueStorage());
  ASSERT(rc == 0);

  auto functionAborted = SpinProcess();

  DEBUG_OUTPUT("Call finished with functionAborted={}", functionAborted);

  if(functionAborted)
  {
    m_historyDuringCall = histGrabber.GetHistoryUntilNow();
    throw RequestInterfaceException(REQ_FUNCTION_ABORTED);
  }

  json doc;
  doc["errorCode"] = {{"value", 0}};
  doc["result"]    = {paramHandler.GetReturnValues()};

  auto passByRef = paramHandler.GetPassByRefInputArray();

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
