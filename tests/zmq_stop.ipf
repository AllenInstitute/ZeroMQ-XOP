#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_stop

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

Function WorksWithoutConnections()

	variable ret

	ret = zeromq_stop()
	CHECK_EQUAL_VAR(ret, 0)
End

Function StopsBinds()

	variable ret

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)

	ret = zeromq_pub_bind("tcp://127.0.0.1:5556")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_stop()
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 0)
End

Function StopsConnections()

	variable ret

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)

	ret = zeromq_client_connect("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_pub_bind("tcp://127.0.0.1:5556")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_sub_connect("tcp://127.0.0.1:5556")
	CHECK_EQUAL_VAR(ret, 0)

	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5556, TCP_V4), 1)

	ret = zeromq_stop()
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 0)
	/// @todo how to check that the connections are closed?
End

// The heartbeat publisher waits 5 s between two heartbeats, stopping it
// must not wait for the end of that
Function StopsHeartbeatPublisherImmediately()

	variable ret, start, elapsed

	ret = zeromq_pub_bind("tcp://127.0.0.1:5556")
	CHECK_EQUAL_VAR(ret, 0)

	// let the heartbeat publisher send its first heartbeat
	Sleep/S 0.5

	start   = stopmstimer(-2)
	ret     = zeromq_stop()
	elapsed = (stopmstimer(-2) - start) / 1e6

	CHECK_EQUAL_VAR(ret, 0)
	CHECK_LT_VAR(elapsed, 1)
End

// There are no IDLE events while a function runs, so the message handler
// can receive requests but not call them
Function WaitWithoutIdleEvents_IGNORE(variable seconds)

	variable start = stopmstimer(-2)

	do
	while((stopmstimer(-2) - start) < (seconds * 1e6))
End

Function CountCalls_IGNORE()

	NVAR/Z numCalls = root:numCalls_IGNORE
	numCalls += 1

	return 0
End

// A request which was received before zeromq_stop() belongs to a closed
// connection and must not be executed afterwards
Function DiscardsRequestsOfClosedConnections()

	variable ret

	string msg = "{\"version\" : 1, \"CallFunction\" : {\"name\" : \"CountCalls_IGNORE\"}}"

	variable/G root:numCalls_IGNORE = 0
	NVAR numCalls = root:numCalls_IGNORE

	zeromq_stop()
	zeromq_set(ZMQ_SET_FLAGS_DEFAULT)

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	ret = zeromq_client_connect("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	ret = zeromq_handler_start()
	CHECK_EQUAL_VAR(ret, 0)

	zeromq_client_send(msg)

	// give the message handler time to receive the request
	WaitWithoutIdleEvents_IGNORE(0.5)

	zeromq_stop()

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	ret = zeromq_handler_start()
	CHECK_EQUAL_VAR(ret, 0)

	DoXOPIdle
	CHECK_EQUAL_VAR(numCalls, 0)

	zeromq_stop()
	KillVariables root:numCalls_IGNORE
End

Function RestartServer_IGNORE()

	NVAR/Z numCalls = root:numCalls_IGNORE
	numCalls += 1

	zeromq_stop()
	zeromq_server_bind("tcp://127.0.0.1:5555")
	zeromq_handler_start()

	return 0
End

// The connection of the caller is closed when the called function calls
// zeromq_stop(), its reply must be discarded without an emergency output
Function DiscardsReplyWhenCalledFunctionRestartsServer()

	variable ret, histRef
	string history

	string msg = "{\"version\" : 1, \"CallFunction\" : {\"name\" : \"RestartServer_IGNORE\"}}"

	variable/G root:numCalls_IGNORE = 0
	NVAR numCalls = root:numCalls_IGNORE

	zeromq_stop()
	zeromq_set(ZMQ_SET_FLAGS_DEFAULT)

	ret = zeromq_server_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	ret = zeromq_client_connect("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	ret = zeromq_handler_start()
	CHECK_EQUAL_VAR(ret, 0)

	histRef = CaptureHistoryStart()

	zeromq_client_send(msg)
	WaitWithoutIdleEvents_IGNORE(0.5)

	// calls RestartServer_IGNORE()
	DoXOPIdle
	CHECK_EQUAL_VAR(numCalls, 1)

	// give the new message handler time to send a queued reply
	WaitWithoutIdleEvents_IGNORE(0.5)

	// outputs the messages of the message handler
	DoXOPIdle

	history = CaptureHistory(histRef, 1)
	CHECK_EQUAL_VAR(GrepString(history, "(?i)emergency"), 0)

	zeromq_stop()
	KillVariables root:numCalls_IGNORE
End
