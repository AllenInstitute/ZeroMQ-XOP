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
