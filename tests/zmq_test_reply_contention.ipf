#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_reply_contention

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

// Regression test for the cross-thread Server-socket contention fix between
// MessageHandler's worker thread and the main thread's reply sending:
Function DoesNotStallRepliesUnderLoad()

	variable ret = zeromq_test_reply_contention(200)
	CHECK_EQUAL_VAR(ret, 0)
End
