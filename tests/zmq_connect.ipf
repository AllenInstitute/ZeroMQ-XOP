#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_connect

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

static Function GetSourceCheck_IGNORE(string type)
	strswitch(type)
		case "CallClientConnect":
			return 0
		case "CallSubConnect":
			return 1
		default:
			FAIL()
	endswitch
End

static Function CallBindCounterPart(string type, string argument)
	strswitch(type)
		case "CallClientConnect":
			return zeromq_server_bind(argument)
		case "CallSubConnect":
			return zeromq_pub_bind(argument)
		default:
			FAIL()
	endswitch
End

static Function/WAVE ConnectLikeFunctions()

	Make/FREE/T funcs = {"CallClientConnect", "CallSubConnect"}

	SetDimLabel 0, 0, zeromq_client_connect, funcs
	SetDimLabel 0, 1, zeromq_sub_connect, funcs

	return funcs
End

Function CONNECT_PROTOTYPE(string argument)
	FAIL()
End

Function CallClientConnect(string argument)
	return zeromq_client_connect(argument)
End

Function CallSubConnect(string argument)
	return zeromq_sub_connect(argument)
End

// UTF_TD_GENERATOR zmq_connect#ConnectLikeFunctions
static Function ComplainsWithInvalidArg1([string str])

	variable err, ret

	FUNCREF CONNECT_PROTOTYPE f = $str

	try
		ret = f(""); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_INVALID_ARG)
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

// UTF_TD_GENERATOR zmq_connect#ConnectLikeFunctions
static Function ComplainsWithInvalidArg2([string str])

	variable err, ret

	FUNCREF CONNECT_PROTOTYPE f = $str

	try
		ret = f("abcd:1234"); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_INVALID_ARG)
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

// UTF_TD_GENERATOR zmq_connect#ConnectLikeFunctions
Function ConnectionsAcceptsHostName([string str])

	variable ret

	FUNCREF CONNECT_PROTOTYPE f = $str

	ret = f("tcp://localhost:5555")
	CHECK_EQUAL_VAR(ret, 0)
End

// UTF_TD_GENERATOR zmq_connect#ConnectLikeFunctions
Function ConnectionOrderDoesNotMatter1([string str])

	variable ret, skipSourceCheck

	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 0)

	FUNCREF CONNECT_PROTOTYPE f = $str
	skipSourceCheck = GetSourceCheck_IGNORE(str)

	ret = f("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	ret = CallBindCounterPart(str, "tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)
End

// UTF_TD_GENERATOR zmq_connect#ConnectLikeFunctions
Function ConnectionOrderDoesNotMatter2([string str])

	variable ret, skipSourceCheck

	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 0)

	FUNCREF CONNECT_PROTOTYPE f = $str
	skipSourceCheck = GetSourceCheck_IGNORE(str)

	ret = CallBindCounterPart(str, "tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	ret = f("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)
End

// UTF_TD_GENERATOR zmq_connect#ConnectLikeFunctions
Function AllowsMultipleConnections([string str])

	variable ret

	FUNCREF CONNECT_PROTOTYPE f = $str

	ret = f("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	ret = f("tcp://127.0.0.1:6666")
	CHECK_EQUAL_VAR(ret, 0)
End
