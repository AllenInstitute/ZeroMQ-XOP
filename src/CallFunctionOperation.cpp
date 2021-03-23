#include "ZeroMQ.h"
#include "CallFunctionOperation.h"
#include "CallFunctionParameterHandler.h"
#include "SerializeWave.h"

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

  ASSERT(rc == 0);

  const auto numParamsSupplied = static_cast<int>(m_params.size());

  if(numParamsSupplied < fip.numRequiredParameters)
  {
    throw RequestInterfaceException(REQ_TOO_FEW_FUNCTION_PARAMS);
  }
  else if(numParamsSupplied > fip.numRequiredParameters)
  {
    throw RequestInterfaceException(REQ_TOO_MANY_FUNCTION_PARAMS);
  }

  // check passed parameters
  for(auto i = 0; i < numParamsSupplied; i += 1)
  {
    if((fip.parameterTypes[i] & NT_CMPLX) == NT_CMPLX)
    {
      throw RequestInterfaceException(REQ_UNSUPPORTED_FUNC_SIG);
    }

    if((fip.parameterTypes[i] & NT_FP64) == NT_FP64)
    {
      if(!IsConvertibleToDouble(m_params[i]))
      {
        throw RequestInterfaceException(REQ_INVALID_PARAM_FORMAT);
      }

      continue;
    }

    if((fip.parameterTypes[i] & HSTRING_TYPE) == HSTRING_TYPE)
    {
      continue;
    }

    if((fip.parameterTypes[i] & DATAFOLDER_TYPE) == DATAFOLDER_TYPE)
    {
      continue;
    }

    throw RequestInterfaceException(REQ_UNSUPPORTED_FUNC_SIG);
  }

  if(fip.returnType != NT_FP64 && fip.returnType != HSTRING_TYPE &&
     fip.returnType != WAVE_TYPE && fip.returnType != DATAFOLDER_TYPE)
  {
    throw RequestInterfaceException(REQ_UNSUPPORTED_FUNC_RET);
  }

  DebugOutput(fmt::format("{}: Request Object can be processed.\r", __func__));
}

json CallFunctionOperation::Call() const
{
  DebugOutput(fmt::format("{}: Data={}.\r", __func__, *this));

  FunctionInfo fip;
  auto rc = GetFunctionInfo(m_name.c_str(), &fip);
  ASSERT(rc == 0);

  CallFunctionParameterHandler p(m_params, fip);

  rc = CallFunction(&fip, p.GetParameterValueStorage(),
                    p.GetReturnValueStorage());
  ASSERT(rc == 0);

  auto functionAborted = SpinProcess();

  DebugOutput(fmt::format("{}: Call finished with functionAborted={}\r",
                          __func__, functionAborted));

  if(functionAborted)
  {
    throw RequestInterfaceException(REQ_FUNCTION_ABORTED);
  }

  json doc;
  doc["errorCode"] = {{"value", 0}};
  doc["result"]    = {p.GetReturnValues()};

  // only serialize the pass-by-ref params if we have some
  if(p.HasPassByRefParameters())
  {
    auto passByRef = p.GetPassByRefArray();

    // we can have optional pass-by-ref structures which we don't support
    if(!passByRef.empty())
    {
      doc["passByReference"] = passByRef;
    }
  }

  return doc;
}
