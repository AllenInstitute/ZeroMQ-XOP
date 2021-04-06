#pragma TextEncoding = "UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_bind

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

static Function GetSourceCheck_IGNORE(string type)
	strswitch(type)
		case "CallServerBind":
			return 0
		case "CallPubBind":
			return 1
		default:
			FAIL()
	endswitch
End

static Function/WAVE BindLikeFunctions()

	Make/FREE/T funcs = {"CallServerBind", "CallPubBind"}

	SetDimLabel 0, 0, zeromq_server_bind, funcs
	SetDimLabel 0, 1, zeromq_pub_bind, funcs

	return funcs
End

Function BIND_PROTOTYPE(string argument)
	FAIL()
End

Function CallServerBind(string argument)
	return zeromq_server_bind(argument)
End

Function CallPubBind(string argument)
	return zeromq_pub_bind(argument)
End

// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function ComplainsWithInvalidArg1([string str])
	variable err, ret

	FUNCREF BIND_PROTOTYPE f = $str

	try
		ret = f(""); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZeroMQ_INVALID_ARG)
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function ComplainsWithInvalidArg2([string str])

	variable err, ret

	FUNCREF BIND_PROTOTYPE f = $str

	try
		ret = f("abcd:1234"); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZeroMQ_INVALID_ARG)
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

// bind does not accept names
Function ComplainsWithInvalidArg3AndServer()

	variable err, ret

	try
		ret = zeromq_server_bind("tcp://localhost:5555"); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZeroMQ_INVALID_ARG)
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

Function WorksWithNamesAndPublisher()

	variable ret

	ret = zeromq_pub_bind("tcp://localhost:5555")
	CHECK_EQUAL_VAR(ret, 0)
End

// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function BindsToLocalHost([string str])

	variable ret, skipSourceCheck

	FUNCREF BIND_PROTOTYPE f = $str
	skipSourceCheck = GetSourceCheck_IGNORE(str)

	ret = f("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
End

// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function BindsToLocalHostIPV6([string str])

	variable ret, skipSourceCheck

	FUNCREF BIND_PROTOTYPE f = $str
	skipSourceCheck = GetSourceCheck_IGNORE(str)

	zeromq_set(ZeroMQ_SET_FLAGS_IPV6)
	ret = f("tcp://::1:5555")
	CHECK_EQUAL_VAR(ret, 0)
End

// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function BindsToLocalHostIPV6AndIPV4([string str])

	variable ret, skipSourceCheck

	FUNCREF BIND_PROTOTYPE f = $str
	skipSourceCheck = GetSourceCheck_IGNORE(str)

	ret = f("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	// the ipv6 flag juggling is required due to https://github.com/zeromq/libzmq/issues/853
	zeromq_set(ZeroMQ_SET_FLAGS_IPV6)
	ret = f("tcp://::1:6666")
	CHECK_EQUAL_VAR(ret, 0)
End

Function ComplainsOnBindOnUsedPortServer()

	variable err, ret

	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 0)
	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)

	try
		ret = zeromq_server_bind("tcp://127.0.0.1:5555"); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZeroMQ_INVALID_ARG)
	endtry
End

Function BindMultipleTimesWorksWithPublisher()

	variable ret

	ret = zeromq_pub_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_pub_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
End

// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function AllowsBindingMultiplePorts([string str])

	variable ret, skipSourceCheck

	FUNCREF BIND_PROTOTYPE f = $str
	skipSourceCheck = GetSourceCheck_IGNORE(str)

	ret = f("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	ret = f("tcp://127.0.0.1:6666")
	CHECK_EQUAL_VAR(ret, 0)
End

Function DoesNotAcceptLargeMessagesOnServerSocket()

	variable err, ret, rc, i
	string identity, reply
	string msg = ""

	zeromq_set(ZeroMQ_SET_FLAGS_DEFAULT | ZeroMQ_SET_FLAGS_NOBUSYWAITRECV)

	rc = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)

	zeromq_client_connect("tcp://127.0.0.1:5555")

	msg = PadString(msg, 1e6, 0x20)

	zeromq_client_send(msg)

	for(i = 0; i < 100; i += 1)
		reply = zeromq_server_recv(identity)
		CHECK_EQUAL_VAR(strlen(identity), 0)
		CHECK_EQUAL_VAR(strlen(reply), 0)
	endfor
End
