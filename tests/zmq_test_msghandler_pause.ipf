#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_msghandler_pause

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

// Regression test for MessageHandlerPauseGuard (SocketWithMutex-adjacent fix
// for the "why does StartZeroMQSockets() sometimes take minutes" slowness):
// the message handler's worker thread polls the Server socket roughly once
// per millisecond, and reconfiguring that same socket (bind, setsockopt)
// while it's running can be starved for libzmq's internal I/O-thread
// attention for an unpredictable, unbounded amount of time. Operations that
// reconfigure the Server socket (ToggleIPV6Support, DoBindOrConnect) now
// stop the handler for their duration via MessageHandlerPauseGuard.
// zeromq_test_msghandler_pause() checks the guard itself actually stops and
// restarts the handler, and that a disabled guard is a true no-op.
Function StopsAndRestartsHandler()

	variable ret = zeromq_test_msghandler_pause()
	CHECK_EQUAL_VAR(ret, 0)
End
