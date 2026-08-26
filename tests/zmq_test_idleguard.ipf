#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_idleguard

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

Function ResetsAfterException()

	variable ret = zeromq_test_idleguard()
	CHECK_EQUAL_VAR(ret, 0)
End
