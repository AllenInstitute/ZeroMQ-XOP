#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_get_bound_endpoints

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

static StrConstant WILDCARD_ENDPOINT_REGEX = "^tcp://127\\.0\\.0\\.1:[0-9]+$"

static Function/WAVE SocketTypes()

	Make/FREE/D types = {ZMQ_SOCKET_TYPE_CLIENT, ZMQ_SOCKET_TYPE_SERVER, ZMQ_SOCKET_TYPE_PUBLISHER, ZMQ_SOCKET_TYPE_SUBSCRIBER}

	SetDimLabel 0, 0, client, types
	SetDimLabel 0, 1, server, types
	SetDimLabel 0, 2, publisher, types
	SetDimLabel 0, 3, subscriber, types

	return types
End

static Function/WAVE InvalidSocketTypes()

	Make/FREE/D types = {0, 5, -1}

	return types
End

static Function/WAVE NonFiniteSocketTypes()

	Make/FREE/D types = {NaN, Inf, -Inf}

	return types
End

static Function CheckEndpoints(variable socketType, WAVE/T expected)

	WAVE/Z/T endpoints = zeromq_get_bound_endpoints(socketType)
	CHECK_WAVE(endpoints, TEXT_WAVE | FREE_WAVE)
	CHECK_EQUAL_TEXTWAVES(endpoints, expected)
End

// UTF_TD_GENERATOR zmq_get_bound_endpoints#SocketTypes
Function ReturnsEmptyWaveWithoutEndpoints([variable var])

	Make/FREE/T/N=0 expected
	CheckEndpoints(var, expected)
End

// UTF_TD_GENERATOR zmq_get_bound_endpoints#InvalidSocketTypes
Function ComplainsWithInvalidSocketType([variable var])

	variable err

	try
		WAVE/Z endpoints = zeromq_get_bound_endpoints(var); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_INVALID_ARG)
	endtry

	CHECK_WAVE(endpoints, NULL_WAVE)
End

// UTF_TD_GENERATOR zmq_get_bound_endpoints#NonFiniteSocketTypes
Function ComplainsWithNonFiniteSocketType([variable var])

	variable err

	try
		WAVE/Z endpoints = zeromq_get_bound_endpoints(var); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CHECK_NEQ_VAR(err, 0)
	endtry

	CHECK_WAVE(endpoints, NULL_WAVE)
End

// The socket type is truncated like the flags of zeromq_set()
Function TruncatesSocketType()

	variable ret

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	Make/FREE/T expected = {"tcp://127.0.0.1:5555"}
	CheckEndpoints(ZMQ_SOCKET_TYPE_SERVER + 0.5, expected)
End

Function ReturnsServerBinds()

	variable ret

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	ret = zeromq_server_bind("tcp://127.0.0.1:5556")
	CHECK_EQUAL_VAR(ret, 0)

	Make/FREE/T expected = {"tcp://127.0.0.1:5555", "tcp://127.0.0.1:5556"}
	CheckEndpoints(ZMQ_SOCKET_TYPE_SERVER, expected)

	Make/FREE/T/N=0 expected
	CheckEndpoints(ZMQ_SOCKET_TYPE_PUBLISHER, expected)
	CheckEndpoints(ZMQ_SOCKET_TYPE_CLIENT, expected)
	CheckEndpoints(ZMQ_SOCKET_TYPE_SUBSCRIBER, expected)
End

Function ReturnsPublisherBinds()

	variable ret

	ret = zeromq_pub_bind("tcp://127.0.0.1:5556")
	CHECK_EQUAL_VAR(ret, 0)

	Make/FREE/T expected = {"tcp://127.0.0.1:5556"}
	CheckEndpoints(ZMQ_SOCKET_TYPE_PUBLISHER, expected)

	Make/FREE/T/N=0 expected
	CheckEndpoints(ZMQ_SOCKET_TYPE_SERVER, expected)
End

Function ReturnsConnections()

	variable ret

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	ret = zeromq_client_connect("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_pub_bind("tcp://127.0.0.1:5556")
	CHECK_EQUAL_VAR(ret, 0)
	ret = zeromq_sub_connect("tcp://127.0.0.1:5556")
	CHECK_EQUAL_VAR(ret, 0)

	Make/FREE/T expected = {"tcp://127.0.0.1:5555"}
	CheckEndpoints(ZMQ_SOCKET_TYPE_CLIENT, expected)

	Make/FREE/T expected = {"tcp://127.0.0.1:5556"}
	CheckEndpoints(ZMQ_SOCKET_TYPE_SUBSCRIBER, expected)
End

// A wildcard bind returns the endpoint with the port chosen by the OS
Function ReturnsResolvedWildcardBind()

	variable ret, port

	ret = zeromq_server_bind("tcp://127.0.0.1:*")
	CHECK_EQUAL_VAR(ret, 0)

	WAVE/Z/T endpoints = zeromq_get_bound_endpoints(ZMQ_SOCKET_TYPE_SERVER)
	CHECK_WAVE(endpoints, TEXT_WAVE | FREE_WAVE)
	CHECK_EQUAL_VAR(DimSize(endpoints, 0), 1)
	CHECK_EQUAL_VAR(GrepString(endpoints[0], WILDCARD_ENDPOINT_REGEX), 1)

	port = str2num(StringFromList(2, endpoints[0], ":"))
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(port, TCP_V4), 1)
End

Function IgnoresFailedBind()

	variable ret, err

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	try
		zeromq_server_bind("tcp://127.0.0.1:5555"); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
	endtry

	Make/FREE/T expected = {"tcp://127.0.0.1:5555"}
	CheckEndpoints(ZMQ_SOCKET_TYPE_SERVER, expected)
End

Function IsClearedByStop()

	variable ret

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	ret = zeromq_client_connect("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	ret = zeromq_pub_bind("tcp://127.0.0.1:5556")
	CHECK_EQUAL_VAR(ret, 0)
	ret = zeromq_sub_connect("tcp://127.0.0.1:5556")
	CHECK_EQUAL_VAR(ret, 0)

	zeromq_stop()

	Make/FREE/T/N=0 expected
	CheckEndpoints(ZMQ_SOCKET_TYPE_CLIENT, expected)
	CheckEndpoints(ZMQ_SOCKET_TYPE_SERVER, expected)
	CheckEndpoints(ZMQ_SOCKET_TYPE_PUBLISHER, expected)
	CheckEndpoints(ZMQ_SOCKET_TYPE_SUBSCRIBER, expected)
End

threadsafe static Function/WAVE GetServerEndpointsInThread()

	return zeromq_get_bound_endpoints(ZMQ_SOCKET_TYPE_SERVER)
End

Function WorksInThreads()

	variable ret, i, numResults

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	Make/FREE/WAVE/N=4 results
	MultiThread results = GetServerEndpointsInThread()

	Make/FREE/T expected = {"tcp://127.0.0.1:5555"}
	numResults = DimSize(results, 0)
	for(i = 0; i < numResults; i += 1)
		WAVE/T endpoints = results[i]
		CHECK_EQUAL_TEXTWAVES(endpoints, expected)
	endfor
End
