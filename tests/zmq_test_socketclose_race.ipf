#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_socketclose_race

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

// The debug output of the many iterations makes writing the JUnit file take very long
static Function TEST_CASE_BEGIN_OVERRIDE(name)
	string name

	zeromq_stop()
	zeromq_set(ZMQ_SET_FLAGS_DEFAULT | ZMQ_SET_FLAGS_LOGGING)
	zeromq_set_interceptor_func("ZeroMQ_Interceptor_Proto")
End

Function DoesNotDeadlockOnConcurrentCloseAndSend()

	variable ret = zeromq_test_socketclose_race()
	CHECK_EQUAL_VAR(ret, 0)
End
