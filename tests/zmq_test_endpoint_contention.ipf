#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_endpoint_contention

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

Function DoesNotStallWithRunningHandler()

	variable ret = zeromq_test_endpoint_contention(200)
	CHECK_EQUAL_VAR(ret, 0)
End
