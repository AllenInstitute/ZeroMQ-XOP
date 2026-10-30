#ifndef ERRORS_H
#define ERRORS_H

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// clang-format off
// XOP error codes
#define OLD_IGOR                     (1 + FIRST_XOP_ERR)
#define UNHANDLED_CPP_EXCEPTION      (2 + FIRST_XOP_ERR)
#define UNKNOWN_SET_FLAG             (3 + FIRST_XOP_ERR)
#define INTERNAL_ERROR               (4 + FIRST_XOP_ERR)
#define INVALID_ARG                  (5 + FIRST_XOP_ERR)
#define HANDLER_ALREADY_RUNNING      (6 + FIRST_XOP_ERR)
#define HANDLER_NO_CONNECTION        (7 + FIRST_XOP_ERR)
#define MISSING_PROCEDURE_FILES      (8 + FIRST_XOP_ERR)
#define INVALID_MESSAGE_FORMAT       (9 + FIRST_XOP_ERR)
#define INVALID_LOGGING_TEMPLATE     (10 + FIRST_XOP_ERR)
#define MESSAGE_FILTER_DUPLICATED    (11 + FIRST_XOP_ERR)
#define MESSAGE_FILTER_MISSING       (12 + FIRST_XOP_ERR)
#define ERR_INVALID_TYPE             (13 + FIRST_XOP_ERR)
#define NO_INTERCEPTOR_FUNC          (14 + FIRST_XOP_ERR)
#define INVALID_INTERCEPTOR_FUNC     (15 + FIRST_XOP_ERR)
#define HEARTBEAT_STARTSTOP_DEADLOCK (16 + FIRST_XOP_ERR)
#define IDLE_GUARD_NOT_RESET         (17 + FIRST_XOP_ERR)
#define QUEUE_APPLY_DEADLOCK         (18 + FIRST_XOP_ERR)
#define SOCKET_CLOSE_RACE_DEADLOCK   (19 + FIRST_XOP_ERR)
#define MSGHANDLER_REPLY_STALL       (20 + FIRST_XOP_ERR)

// non-XOP error codes

/// @name Error codes for the request JSON interface
/// @{
constexpr int REQ_SUCCESS                    = 0;
constexpr int REQ_UNKNOWN_ERROR              = 1;
constexpr int REQ_INVALID_JSON_OBJECT        = 3;
constexpr int REQ_INVALID_VERSION            = 4;
constexpr int REQ_INVALID_OPERATION          = 5;
constexpr int REQ_INVALID_OPERATION_FORMAT   = 6;
constexpr int REQ_INVALID_MESSAGEID          = 7;
constexpr int REQ_OUT_OF_MEMORY              = 8;
/// @name Error codes for the CallFunction class
/// @{
constexpr int REQ_PROC_NOT_COMPILED        = 100;
constexpr int REQ_NON_EXISTING_FUNCTION    = 101;
constexpr int REQ_TOO_FEW_FUNCTION_PARAMS  = 102;
constexpr int REQ_TOO_MANY_FUNCTION_PARAMS = 103;
constexpr int REQ_UNSUPPORTED_FUNC_SIG     = 104;
constexpr int REQ_UNSUPPORTED_FUNC_RET     = 105;
constexpr int REQ_INVALID_PARAM_FORMAT     = 106;
constexpr int REQ_FUNCTION_ABORTED         = 107;
constexpr int REQ_INTERCEPT_FUNC_ABORTED   = 108;
/// @}
/// @}
// clang-format on

#endif // ERRORS_H
