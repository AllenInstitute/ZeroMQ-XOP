#pragma TextEncoding = "UTF-8"
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
