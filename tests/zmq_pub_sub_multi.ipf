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

static Function [WAVE/T elem0, WAVE/T elem1] MinimalSetupPub()

	int ret

	Make/FREE/WAVE/N=(2) contents

	Make/FREE/N=(1)/T elem0 = "abcd"
	contents[0] = elem0

	Make/FREE/N=(1)/T elem1 = "hi there!"
	contents[1] = elem1

	ret = zeromq_sub_remove_filter("")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_sub_add_filter("abcd")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_pub_send_multi(contents)
	CHECK_EQUAL_VAR(ret, 0)

	return [elem0, elem1]
End

static Function ReceiveChecksErrors()

	int ret

	Init_IGNORE()

	WAVE/T elem0, elem1
	[elem0, elem1] = MinimalSetupPub()

	// null wave
	try
		ret = zeromq_sub_recv_multi($""); AbortOnRTE
		FAIL()
	catch
		CHECK_RTE(ERR_NOWAV)
	endtry

	[elem0, elem1] = MinimalSetupPub()

	// wrong container wave type
	try
		Make/FREE wvFloat
		ret = zeromq_sub_recv_multi(wvFloat); AbortOnRTE
		FAIL()
	catch
		CheckErrorMessage(GetRTError(0), ZMQ_MESSAGE_INVALID_TYPE)
		CHECK_ANY_RTE()
	endtry

	[elem0, elem1] = MinimalSetupPub()

	// contained wave has the wrong type (WAVE)
	try
		Make/FREE/WAVE/N=2 wv
		Make/FREE/DF wvWave
		wv[0] = wvWave
		ret = zeromq_sub_recv_multi(wv); AbortOnRTE
		FAIL()
	catch
		CheckErrorMessage(GetRTError(0), ZMQ_MESSAGE_INVALID_TYPE)
		CHECK_ANY_RTE()
	endtry

	[elem0, elem1] = MinimalSetupPub()

	// first and second contained wave have to be text
	try
		Make/FREE/WAVE/N=2 wv
		Make/FREE/D wvDouble
		wv[0] = wvDouble
		ret = zeromq_sub_recv_multi(wv); AbortOnRTE
		FAIL()
	catch
		CheckErrorMessage(GetRTError(0), ZMQ_MESSAGE_INVALID_TYPE)
		CHECK_ANY_RTE()
	endtry
End

static Function ReceiveWithOnlyTwoWavesWorks()

	int ret

	Init_IGNORE()

	WAVE/T elem0, elem1
	[elem0, elem1] = MinimalSetupPub()

	Make/WAVE/N=0 contentsReceived
	ret = zeromq_sub_recv_multi(contentsReceived)
	CHECK_EQUAL_VAR(ret, 0)

	CHECK_EQUAL_VAR(DimSize(contentsReceived, 0), 2)
	CHECK_EQUAL_WAVES(contentsReceived[0], elem0)
	CHECK_EQUAL_WAVES(contentsReceived[1], elem1)
End

static Function ReceiveWithThreeTextWavesWorks()

	int ret

	Init_IGNORE()

	Make/FREE/WAVE/N=(3) contents

	Make/FREE/N=(1)/T elem0 = "abcd"
	contents[0] = elem0

	Make/FREE/N=(1)/T elem1 = "hi there!"
	contents[1] = elem1

	Make/FREE/N=(1)/T elem2 = "and again"
	contents[2] = elem2

	ret = zeromq_sub_add_filter("abcd")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_pub_send_multi(contents)
	CHECK_EQUAL_VAR(ret, 0)

	// we need to feed in three text waves initally so that we also get
	// a text wave for the third one
	Make/WAVE/N=(3) contentsReceived
	Make/FREE/N=(1)/T elemReceived0, elemReceived1, elemReceived2

	contentsReceived[0] = elemReceived0
	contentsReceived[1] = elemReceived1
	contentsReceived[2] = elemReceived2

	ret = zeromq_sub_recv_multi(contentsReceived)
	CHECK_EQUAL_VAR(ret, 0)

	CHECK_EQUAL_VAR(DimSize(contentsReceived, 0), 3)
	CHECK_EQUAL_WAVES(contentsReceived[0], elem0)
	CHECK_EQUAL_WAVES(contentsReceived[1], elem1)
	CHECK_EQUAL_WAVES(contentsReceived[2], elem2)
End

static Function ReceiveTextAndFloatsWorks()

	int ret

	Init_IGNORE()

	Make/FREE/WAVE/N=(3) contents

	Make/FREE/N=(1)/T elem0 = "abcd"
	contents[0] = elem0

	Make/FREE/N=(1)/T elem1 = "hi there!"
	contents[1] = elem1

	Make/FREE/N=(2, 3)/D elem2 = (p + q)^2
	contents[2] = elem2

	ret = zeromq_sub_add_filter("abcd")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_pub_send_multi(contents)
	CHECK_EQUAL_VAR(ret, 0)

	Make/WAVE/N=(0)/FREE contentsReceived
	ret = zeromq_sub_recv_multi(contentsReceived)
	CHECK_EQUAL_VAR(ret, 0)

	CHECK_EQUAL_VAR(DimSize(contentsReceived, 0), 3)
	CHECK_EQUAL_WAVES(contentsReceived[0], elem0)
	CHECK_EQUAL_WAVES(contentsReceived[1], elem1)
	CHECK_WAVE(contentsReceived[2], NUMERIC_WAVE | FREE_WAVE, minorType = INT8_WAVE | UNSIGNED_WAVE)

	WAVE wv = contentsReceived[2]
	Redimension/N=(2, 3)/E=1/D wv
	CHECK_EQUAL_WAVES(wv, elem2)
End

static Function ReceiveTextAndFloatsPreExistingWorks()

	int ret

	Init_IGNORE()

	Make/FREE/WAVE/N=(3) contents

	Make/FREE/N=(1)/T elem0 = "abcd"
	contents[0] = elem0

	Make/FREE/N=(1)/T elem1 = "hi there!"
	contents[1] = elem1

	Make/FREE/N=(2, 3)/D elem2 = (p + q)^2
	contents[2] = elem2

	ret = zeromq_sub_add_filter("abcd")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_pub_send_multi(contents)
	CHECK_EQUAL_VAR(ret, 0)

	Make/WAVE/N=(3)/FREE contentsReceived
	Make/FREE/N=(1)/T elemReceived0, elemReceived1
	Make/FREE/D elemReceived2

	contentsReceived[0] = elemReceived0
	contentsReceived[1] = elemReceived1
	contentsReceived[2] = elemReceived2

	ret = zeromq_sub_recv_multi(contentsReceived)
	CHECK_EQUAL_VAR(ret, 0)

	CHECK_EQUAL_VAR(DimSize(contentsReceived, 0), 3)
	CHECK_EQUAL_WAVES(contentsReceived[0], elem0)
	CHECK_EQUAL_WAVES(contentsReceived[1], elem1)
	CHECK_WAVE(contentsReceived[2], NUMERIC_WAVE | FREE_WAVE, minorType = DOUBLE_WAVE)

	WAVE wv = contentsReceived[2]
	Redimension/N=(2, 3)/E=1 wv
	CHECK_EQUAL_WAVES(wv, elem2)
End
