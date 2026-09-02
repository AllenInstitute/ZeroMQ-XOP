#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_queueswap

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

// Regression test for ConcurrentQueue::apply_to_all's queue-swap fix
Function DoesNotDeadlockOnReentrantPush()

	variable ret = zeromq_test_queueswap()
	CHECK_EQUAL_VAR(ret, 0)
End
