#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_socketclose_race

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

// Regression test for the GET_SOCKET/SocketWithMutex fetch-then-relock race
// (GlobalData::GetOrCreateSocket, SocketWithMutex.h): before the fix, one
// thread could fetch a socket pointer, have another thread close that exact
// socket before the first thread's own lock was (re-)acquired, and end up
// using the now-stale pointer -- this is what caused Igor Pro to freeze when
// a caller (e.g. MIES's StartZeroMQSockets()) repeatedly closed and rebound
// sockets while HeartbeatPublisher's always-running background thread was
// concurrently sending on them. zeromq_test_socketclose_race() reproduces
// that exact interaction directly and throws
// ZMQ_SOCKET_CLOSE_RACE_DEADLOCK if it does not complete in time.
Function DoesNotDeadlockOnConcurrentCloseAndSend()

	variable ret = zeromq_test_socketclose_race()
	CHECK_EQUAL_VAR(ret, 0)
End
