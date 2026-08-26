#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_idleguard

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

// Regression test for the IdleGuard fix in ZeroMQ.cpp's XOPEntry IDLE case:
// without it, an exception escaping the guarded block left idleInProgress
// stuck at `true` forever, permanently disabling all further IDLE
// processing (queued RPC replies and history notices would never flush
// again). zeromq_test_idleguard() exercises IdleGuard the same way XOPEntry
// does and throws ZMQ_IDLE_GUARD_NOT_RESET if idleInProgress did not reset.
Function ResetsAfterException()

	variable ret = zeromq_test_idleguard()
	CHECK_EQUAL_VAR(ret, 0)
End
