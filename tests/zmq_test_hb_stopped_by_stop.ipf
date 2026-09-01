#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_hb_stopped_by_stop

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

Function StopsHeartbeatPublisher()

	variable ret = zeromq_test_hb_stopped_by_stop()
	CHECK_EQUAL_VAR(ret, 0)
End
