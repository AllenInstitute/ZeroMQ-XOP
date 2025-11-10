#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_set_interceptor_func

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

static Function/WAVE NonExistingInterceptorFunctions()

	Make/T/FREE funcs = {"", "I_DONT_EXIST", "123"}

	return funcs
End

static Function/WAVE InvalidInterceptorFunctions()

	Make/T/FREE funcs = {"InterceptTooFewParams", "InterceptTooManyParams", "InterceptWrongParamType", "InterceptWrongReturnType"}

	return funcs
End

Function InterceptTooFewParams(string str)
End

Function InterceptTooManyParams(string str1, string str2, string str3, string str4)
End

Function InterceptWrongParamType(string str1, variable var)
End

Function/S InterceptWrongReturnType(string str1, string str2)
End

static Function/WAVE ValidInterceptorFunctions()

	// ZeroMQ_Interceptor_Proto does abort but we are not executing it here
	Make/T/FREE funcs = {"ZeroMQ_Interceptor_Proto", "zmq_set_interceptor_func#MyInterceptor", "MyInterceptorPublic"}

	return funcs
End

Function MyInterceptorPublic(string json, string identity, variable mode)

	return MyInterceptor(json, identity, mode)
End

static Function MyInterceptor(string json, string identity, variable mode)

	printf "json=%s\r", json
	printf "identity=%s\r", identity
	printf "mode=%g\r", mode
End

// UTF_TD_GENERATOR zmq_set_interceptor_func#NonExistingInterceptorFunctions
Function NonExistingInterceptFunctionName([string str])

	variable err

	try
		zeromq_set_interceptor_func(str); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_NO_INTERCEPTOR_FUNC)
	endtry
End

// UTF_TD_GENERATOR zmq_set_interceptor_func#InvalidInterceptorFunctions
Function InvalidInterceptSignature([string str])

	variable err

	try
		zeromq_set_interceptor_func(str); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_INVALID_INTERCEPTOR_FUNC)
	endtry
End

// UTF_TD_GENERATOR zmq_set_interceptor_func#ValidInterceptorFunctions
Function ValidInterceptSignature([string str])

	zeromq_set_interceptor_func(str)
	CHECK_NO_RTE()
End

Function InterceptorIsCalledWhenEnabled()

	variable err, ret, errorValue, resultVariable
	variable expected, histRef
	string   replyMessage, history, historyFull, expectedStr

	string msg = "{                    "              + \
	             "\"version\" : 1,                  " + \
	             "\"CallFunction\" : {             "  + \
	             "\"name\" : \"FunctionToCall\"  "    + \
	             "}                                 " + \
	             "}"

	zeromq_stop()
	zeromq_server_bind("tcp://127.0.0.1:5555")
	zeromq_client_connect("tcp://127.0.0.1:5555")

	zeromq_set(ZeroMQ_SET_FLAGS_INTERCEPTOR)
	zeromq_set_interceptor_func("MyInterceptorPublic")
	CHECK_NO_RTE()

	ret = zeromq_handler_start()
	CHECK_EQUAL_VAR(ret, 0)

	zeromq_client_send(msg)

	histRef = CaptureHistoryStart()

	// the json message is now in the internal message queue and
	// will be processed at the next idle event
	// zeromq_recv will also create idle events while waiting
	replyMessage = zeromq_client_recv()

	historyFull = CaptureHistory(histRef, 1)
	history = GrepList(historyFull, "^(json|identity|mode)=", 0, "\r")

	CHECK_EQUAL_STR(history, "json={\"name\":\"FunctionToCall\"}\ridentity=zeromq xop: dealer\rmode=1\rjson={\"name\":\"FunctionToCall\"}\ridentity=zeromq xop: dealer\rmode=2\r")

	history = GrepList(historyFull, "^(mode)=", 0, "\r")
	sprintf expectedStr, "mode=%d\rmode=%d\r", ZeroMQ_INTERCEPT_BEGIN, ZeroMQ_INTERCEPT_END
	CHECK_EQUAL_STR(history, expectedStr)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	expected = FunctionToCall()
	CHECK_EQUAL_VAR(resultVariable, expected)
End

Function DoAbort_IGNORE()

	Abort
End

Function InterceptorIsCalledWithAbortingFunction()

	variable err, ret, errorValue, resultVariable
	variable expected, histRef
	string   replyMessage, history, historyFull, expectedStr

	string msg = "{                    "              + \
	             "\"version\" : 1,                  " + \
	             "\"CallFunction\" : {             "  + \
	             "\"name\" : \"DoAbort_IGNORE\"     " + \
	             "}                                 " + \
	             "}"

	zeromq_stop()
	zeromq_server_bind("tcp://127.0.0.1:5555")
	zeromq_client_connect("tcp://127.0.0.1:5555")

	zeromq_set(ZeroMQ_SET_FLAGS_INTERCEPTOR)
	zeromq_set_interceptor_func("MyInterceptorPublic")
	CHECK_NO_RTE()

	ret = zeromq_handler_start()
	CHECK_EQUAL_VAR(ret, 0)

	zeromq_client_send(msg)

	histRef = CaptureHistoryStart()

	// the json message is now in the internal message queue and
	// will be processed at the next idle event
	// zeromq_recv will also create idle events while waiting
	try
		replyMessage = zeromq_client_recv()
		FAIL()
	catch
		PASS()
	endtry

	historyFull = CaptureHistory(histRef, 1)
	history = GrepList(historyFull, "^(json|identity|mode)=", 0, "\r")

	// due to an Igor Pro implementation detail
	// we don't get a ZeroMQ_INTERCEPT_END call for the interceptor if the called function aborts

	CHECK_EQUAL_STR(history, "json={\"name\":\"DoAbort_IGNORE\"}\ridentity=zeromq xop: dealer\rmode=1\r")

	history = GrepList(historyFull, "^(mode)=", 0, "\r")
	sprintf expectedStr, "mode=%d\r", ZeroMQ_INTERCEPT_BEGIN
	CHECK_EQUAL_STR(history, expectedStr)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_FUNCTION_ABORTED)
End

Function AbortingInterceptor(string json, string identity, variable mode)

	Abort
End

Function AbortingInterceptorIsHandled()

	variable err, ret, errorValue, resultVariable
	variable expected, histRef
	string   replyMessage, history

	string msg = "{                    "              + \
	             "\"version\" : 1,                  " + \
	             "\"CallFunction\" : {             "  + \
	             "\"name\" : \"FunctionToCall\"  "    + \
	             "}                                 " + \
	             "}"

	zeromq_stop()
	zeromq_server_bind("tcp://127.0.0.1:5555")
	zeromq_client_connect("tcp://127.0.0.1:5555")

	zeromq_set(ZeroMQ_SET_FLAGS_INTERCEPTOR)
	zeromq_set_interceptor_func("AbortingInterceptor")
	CHECK_NO_RTE()

	ret = zeromq_handler_start()
	CHECK_EQUAL_VAR(ret, 0)

	zeromq_client_send(msg)

	// the json message is now in the internal message queue and
	// will be processed at the next idle event
	// zeromq_recv will also create idle events while waiting
	//
	// interceptor function is aborting
	// this is very weird as we are aborting but also the reply value is set
	try
		replyMessage = zeromq_client_recv()
	catch
		CHECK_EQUAL_VAR(V_AbortCode, -3)
		errorValue = ExtractErrorValue(replyMessage)
		CHECK_EQUAL_VAR(errorValue, REQ_INTERCEPT_FUNC_ABORTED)
	endtry
End
