#pragma once

#include <ctime>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <thread>
#include <numeric>
#include <mutex>

#include "zmq.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "XOPStandardHeaders.h" // Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

// Usign std::min/max
#undef min
#undef max

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

#ifdef MACIGOR64
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wtautological-overlap-compare"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wc++17-extensions"
#pragma clang diagnostic ignored "-Wweak-vtables"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#ifdef MACIGOR64
#pragma clang diagnostic pop
#endif

class CallFunctionOperation;
using CallFunctionOperationPtr = std::shared_ptr<CallFunctionOperation>;

class RequestInterface;
using RequestInterfacePtr = std::shared_ptr<RequestInterface>;

using StringVector = std::vector<std::string>;

using LockGuard = std::lock_guard<std::recursive_mutex>;

#ifdef MACIGOR64
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#pragma clang diagnostic ignored "-Wheader-hygiene"
#pragma clang diagnostic ignored "-Wweak-vtables"
#pragma clang diagnostic ignored "-Wunused-member-function"
#pragma clang diagnostic ignored "-Wsigned-enum-bitfield"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#pragma clang diagnostic ignored "-Wc++2a-compat"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4018)
#pragma warning(disable : 4702)
#endif

#include <fmt/format.h>
#include <fmt/printf.h>

using namespace fmt::literals; // NOLINT

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef MACIGOR64
#pragma clang diagnostic pop
#endif

#include "cmake_config.h"
#include "functions.h"
#include "GlobalData.h"
#include "CustomExceptions.h"
#include "RequestInterfaceException.h"
#include "HelperFunctions.h"
#include "ConcurrentXOPNotice.h"
#include "SocketWithMutex.h"
#include "Errors.h"
#include "git_version.h"

// see also FunctionInfo XOPSupport function
const int MAX_NUM_PARAMS        = 100;
const std::string MESSAGEID_KEY = "messageID";

/* Prototypes */
HOST_IMPORT int XOPMain(IORecHandle ioRecHandle);
