#include "ZeroMQ.h"
#include "CallFunctionOperation.h"
#include "CallFunctionParameterHandler.h"
#include "SerializeWave.h"
#include <algorithm>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

#ifdef MACIGOR

namespace std
{

#endif

std::ostream &operator<<(std::ostream &out, std::vector<std::string> vec)
{
  fmt::fprintf(out, "%s", "[");

  for(auto it = vec.cbegin(); it != vec.cend(); it++)
  {
    if(std::distance(it, vec.cend()) > 1)
    {
      fmt::fprintf(out, "%s, ", *it);
    }
    else
    {
      fmt::fprintf(out, "%s", *it);
    }
  }

  fmt::fprintf(out, "%s", "]");

  return out;
}

#ifdef MACIGOR

} // namespace std

#endif

std::ostream &operator<<(std::ostream &out, CallFunctionOperation op)
{
  fmt::fprintf(out, "name=%s, params=%s", op.m_name, op.m_params);
  return out;
}

CallFunctionOperation::CallFunctionOperation(json j)
{
  DebugOutput(fmt::sprintf("%s: size=%d\r", __func__, j.size()));

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
      fmt::sprintf("%s: CallFunction object could be created.\r", __func__));
}

void CallFunctionOperation::CanBeProcessed() const
{
  DebugOutput(fmt::sprintf("%s: Data=%s.\r", __func__, *this));

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

  XOPNotice(
      fmt::sprintf("%s: func return value is =%d.\r", __func__, fip.returnType)
          .c_str());

  ASSERT(rc == 0);

  const auto numReturnValues = GetNumberOfReturnValues(fip);
  const auto numInputParams  = GetNumberOfInputParameters(fip, numReturnValues);
  const auto multipleReturnValueSyntax = UsesMultipleReturnValueSyntax(fip);

  const auto numParamsSupplied = static_cast<int>(m_params.size());

  DebugOutput(fmt::sprintf(
      "%s: Multiple return value syntax=%d, Number of return values=%d, Number "
      "of required input parameters = %d, Number of parmeters supplied = %d\r",
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

  // check passed parameters
  for(auto i = firstInputParamIndex; i < numParamsSupplied; i += 1)
  {
    auto igorType = fip.parameterTypes[i];

    if(IsBitSet(igorType, NT_FP64))
    {
      char *lastChar;
      std::strtod(m_params[i].c_str(), &lastChar);

      if(*lastChar != '\0')
      {
        throw RequestInterfaceException(REQ_INVALID_PARAM_FORMAT);
      }

      continue;
    }
    else if(multipleReturnValueSyntax && IsBitSet(igorType, FV_REF_TYPE))
    {
      throw RequestInterfaceException(REQ_UNSUPPORTED_FUNC_SIG);
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

  // fixme check output parameters when multiple return value syntax is used

  if(fip.returnType != NT_FP64 && fip.returnType != HSTRING_TYPE &&
     fip.returnType != WAVE_TYPE && fip.returnType != DATAFOLDER_TYPE &&
     fip.returnType != FV_NORETURN_TYPE)
  {
    throw RequestInterfaceException(REQ_UNSUPPORTED_FUNC_RET);
  }

  DebugOutput(fmt::sprintf("%s: Request Object can be processed.\r", __func__));
}

json CallFunctionOperation::Call() const
{
  DebugOutput(fmt::sprintf("%s: Data=%s.\r", __func__, *this));

  FunctionInfo fip;
  auto rc = GetFunctionInfo(m_name.c_str(), &fip);
  ASSERT(rc == 0);

  CallFunctionParameterHandler p(m_params, fip);

  rc = CallFunction(&fip, p.GetParameterValueStorage(),
                    p.GetReturnValueStorage());
  ASSERT(rc == 0);

  auto functionAborted = SpinProcess();

  DebugOutput(fmt::sprintf("%s: Call finished with functionAborted=%d\r",
                           __func__, functionAborted));

  if(functionAborted)
  {
    throw RequestInterfaceException(REQ_FUNCTION_ABORTED);
  }

  json doc;
  doc["errorCode"] = {{"value", 0}};
  doc["result"]    = {p.GetReturnValues()};

  auto passByRef = p.GetPassByRefInputArray();

  // we can have optional pass-by-ref structures which we don't support
  // FIXME check
  if(!passByRef.empty())
  {
    doc["passByReference"] = {passByRef};
  }

  return doc;
}
