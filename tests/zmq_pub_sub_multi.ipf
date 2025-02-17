#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_pub_sub_multi

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

static Constant ERR_NOWAV = 2

static Function Init_IGNORE()

	variable ret

	zeromq_set(ZMQ_SET_FLAGS_DEBUG | ZMQ_SET_FLAGS_DEFAULT | ZMQ_SET_FLAGS_LOGGING | ZMQ_SET_FLAGS_NOBUSYWAITRECV)

	ret = zeromq_pub_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_sub_connect("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)
End

static Function SendChecksInput()

	variable ret

	// null wave
	try
		ret = zeromq_pub_send_multi($""); AbortOnRTE
		FAIL()
	catch
		CHECK_RTE(ERR_NOWAV)
	endtry

	// wrong wave type
	try
		Make/FREE wvFloat
		ret = zeromq_pub_send_multi(wvFloat); AbortOnRTE
		FAIL()
	catch
		CheckErrorMessage(GetRTError(0), ZMQ_MESSAGE_INVALID_TYPE)
		CHECK_ANY_RTE()
	endtry

	// wrong number of rows
	try
		Make/FREE/WAVE/N=1 wv
		ret = zeromq_pub_send_multi(wv); AbortOnRTE
		FAIL()
	catch
		CheckErrorMessage(GetRTError(0), ZMQ_MESSAGE_INVALID_TYPE)
		CHECK_ANY_RTE()
	endtry

	// wrong number of cols
	try
		Make/FREE/WAVE/N=(2, 2) wv
		ret = zeromq_pub_send_multi(wv); AbortOnRTE
		FAIL()
	catch
		CheckErrorMessage(GetRTError(0), ZMQ_MESSAGE_INVALID_TYPE)
		CHECK_ANY_RTE()
	endtry

	// contained wave is null
	try
		Make/FREE/WAVE/N=2 wv
		wv[0] = $""
		ret = zeromq_pub_send_multi(wv); AbortOnRTE
		FAIL()
	catch
		CHECK_RTE(ERR_NOWAV)
	endtry

	// contained wave has the wrong type (DFREF)
	try
		Make/FREE/WAVE/N=2 wv
		Make/FREE/DF wvDF
		wv[0] = wvDF
		ret = zeromq_pub_send_multi(wv); AbortOnRTE
		FAIL()
	catch
		CheckErrorMessage(GetRTError(0), ZMQ_MESSAGE_INVALID_TYPE)
		CHECK_ANY_RTE()
	endtry

	// contained wave has the wrong type (WAVE)
	try
		Make/FREE/WAVE/N=2 wv
		Make/FREE/DF wvWave
		wv[0] = wvWave
		ret = zeromq_pub_send_multi(wv); AbortOnRTE
		FAIL()
	catch
		CheckErrorMessage(GetRTError(0), ZMQ_MESSAGE_INVALID_TYPE)
		CHECK_ANY_RTE()
	endtry

	// second contained wave has the wrong type
	try
		Make/FREE/WAVE/N=2 wv
		Make/FREE/T/N=(1) wvText
		Make/FREE wvFloat
		wv[0] = wvText
		wv[1] = wvFloat
		ret = zeromq_pub_send_multi(wv); AbortOnRTE
		FAIL()
	catch
		CheckErrorMessage(GetRTError(0), ZMQ_MESSAGE_INVALID_TYPE)
		CHECK_ANY_RTE()
	endtry

	// text waves have the wrong size
	try
		Make/FREE/WAVE/N=2 wv
		Make/FREE/T/N=(0) wvText
		wv[0] = wvText
		ret = zeromq_pub_send_multi(wv); AbortOnRTE
		FAIL()
	catch
		CheckErrorMessage(GetRTError(0), ZMQ_MESSAGE_INVALID_TYPE)
		CHECK_ANY_RTE()
	endtry
End

static Function SendingWithOnlyTwoWavesWorks()

	int ret, i
	string msg, filter

	Init_IGNORE()

	Make/FREE/WAVE/N=(2) contents

	Make/FREE/N=(1)/T elem0 = "abcd"
	contents[0] = elem0

	Make/FREE/N=(1)/T elem1 = "hi there!"
	contents[1] = elem1

	ret = zeromq_sub_add_filter("abcd")

	ret = zeromq_pub_send_multi(contents)
	CHECK_EQUAL_VAR(ret, 0)

	msg = zeromq_sub_recv(filter)
	CHECK_EQUAL_STR(filter, elem0[0])
	CHECK_EQUAL_STR(msg, elem1[0])
End
