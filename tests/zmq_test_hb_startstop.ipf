#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_hb_startstop

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

// Regression test for a race between HeartbeatPublisher::Start() and Stop():
Function DoesNotDeadlockOnRapidStartStop()

	variable ret = zeromq_test_hb_startstop(500)
	CHECK_EQUAL_VAR(ret, 0)
End
