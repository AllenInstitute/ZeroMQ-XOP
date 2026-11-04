#pragma TextEncoding="UTF-8"
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
		CheckErrorMessage(err, ZMQ_INVALID_ARG)
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
		CheckErrorMessage(err, ZMQ_INVALID_ARG)
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

// bind does not accept names
// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function ComplainsWithInvalidArg3([string str])

	variable err, ret

	FUNCREF BIND_PROTOTYPE f = $str

	try
		ret = f("tcp://localhost:5555"); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_INVALID_ARG)
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function BindsToLocalHost([string str])

	variable ret, skipSourceCheck

	FUNCREF BIND_PROTOTYPE f = $str
	skipSourceCheck = GetSourceCheck_IGNORE(str)

	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 0)

	ret = f("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)
End

// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function BindsToLocalHostIPV6([string str])

	variable ret, skipSourceCheck

	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V6), 0)

	FUNCREF BIND_PROTOTYPE f = $str
	skipSourceCheck = GetSourceCheck_IGNORE(str)

	zeromq_set(ZMQ_SET_FLAGS_IPV6)
	ret = f("tcp://::1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V6), 1)
End

// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function BindsToLocalHostIPV6AndIPV4([string str])

	variable ret, skipSourceCheck

	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(6666, TCP_V6), 0)

	FUNCREF BIND_PROTOTYPE f = $str
	skipSourceCheck = GetSourceCheck_IGNORE(str)

	ret = f("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)

	// the ipv6 flag juggling is required due to https://github.com/zeromq/libzmq/issues/853
	zeromq_set(ZMQ_SET_FLAGS_IPV6)
	ret = f("tcp://::1:6666")
	CHECK_EQUAL_VAR(ret, 0)

	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(6666, TCP_V6), 1)
End

// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function ComplainsOnBindOnUsedPortServer([string str])

	variable err, ret

	FUNCREF BIND_PROTOTYPE f = $str

	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 0)
	ret = f("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)

	try
		ret = f("tcp://127.0.0.1:5555"); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_INVALID_ARG)
	endtry
End

// UTF_TD_GENERATOR zmq_bind#BindLikeFunctions
Function AllowsBindingMultiplePorts([string str])

	variable ret, skipSourceCheck

	FUNCREF BIND_PROTOTYPE f = $str
	skipSourceCheck = GetSourceCheck_IGNORE(str)

	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 0)
	ret = f("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)

	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(6666, TCP_V4), 0)
	ret = f("tcp://127.0.0.1:6666")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(6666, TCP_V4), 1)
End

static Constant MAX_SERVER_MESSAGE_SIZE = 65536

static Function/S ReceiveOnServer_IGNORE(variable timeout)

	variable start = stopmstimer(-2)
	string identity
	string msg = ""

	do
		msg = zeromq_server_recv(identity)
		if(strlen(msg) > 0)
			break
		endif
	while((stopmstimer(-2) - start) < (timeout * 1e6))

	return msg
End

Function AcceptsMessagesUpToTheSizeLimitOnServerSocket()

	variable ret
	string reply
	string msg = ""

	zeromq_set(ZMQ_SET_FLAGS_DEFAULT | ZMQ_SET_FLAGS_NOBUSYWAITRECV)

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	zeromq_client_connect("tcp://127.0.0.1:5555")

	msg = PadString(msg, MAX_SERVER_MESSAGE_SIZE, 0x20)

	zeromq_client_send(msg)

	reply = ReceiveOnServer_IGNORE(1)
	CHECK_EQUAL_STR(reply, msg)
End

// libzmq closes the connection of a client which sends a too large message,
// the client reconnects automatically
Function DropsLargerMessagesOnServerSocket()

	variable ret
	string reply
	string msg = ""

	zeromq_set(ZMQ_SET_FLAGS_DEFAULT | ZMQ_SET_FLAGS_NOBUSYWAITRECV)

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	zeromq_client_connect("tcp://127.0.0.1:5555")

	msg = PadString(msg, MAX_SERVER_MESSAGE_SIZE + 1, 0x20)

	zeromq_client_send(msg)

	reply = ReceiveOnServer_IGNORE(1)
	CHECK_EMPTY_STR(reply)

	zeromq_client_send("abcd")

	reply = ReceiveOnServer_IGNORE(5)
	CHECK_EQUAL_STR(reply, "abcd")
End

Function AllowsEmptyIdentityAndPayload()

	string reply, identity

	zeromq_stop()
	zeromq_server_bind("tcp://127.0.0.1:5555")

	zeromq_client_connect("tcp://127.0.0.1:5555")

	zeromq_client_send("")

	reply = zeromq_server_recv(identity)

	zeromq_server_send(identity, "")
	CHECK_NO_RTE()
End
