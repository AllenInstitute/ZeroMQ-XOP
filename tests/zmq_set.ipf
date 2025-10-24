#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zqm_set

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

Function ComplainsWithUnknownFlagsLow()

	variable err, ret

	try
		ret = zeromq_set(-1); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_UNKNOWN_SET_FLAG)
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

Function ComplainsWithUnknownFlagsHigh()

	variable err, ret

	try
		ret = zeromq_set(64); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_UNKNOWN_SET_FLAG)
		PASS()
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

Function AcceptsDefaultFlag()

	variable ret, err

	try
		ret = zeromq_set(ZMQ_SET_FLAGS_DEFAULT); AbortOnRTE
		PASS()
	catch
		err = GetRTError(1)
		FAIL()
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

Function AcceptsDebugFlag()

	variable ret, err

	try
		ret = zeromq_set(ZMQ_SET_FLAGS_DEBUG); AbortOnRTE
		PASS()
	catch
		err = GetRTError(1)
		FAIL()
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

Function AcceptsIPV6Flag()

	variable ret, err

	try
		ret = zeromq_set(ZMQ_SET_FLAGS_IPV6); AbortOnRTE
		PASS()
	catch
		err = GetRTError(1)
		FAIL()
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

Function AcceptsRecvWaitFlag()

	variable ret, err

	try
		ret = zeromq_set(ZMQ_SET_FLAGS_NOBUSYWAITRECV); AbortOnRTE
		PASS()
	catch
		err = GetRTError(1)
		FAIL()
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End

Function AcceptsMultipleFLags()

	variable ret, err

	try
		ret = zeromq_set(ZMQ_SET_FLAGS_IPV6 | ZMQ_SET_FLAGS_DEBUG); AbortOnRTE
		PASS()
	catch
		err = GetRTError(1)
		FAIL()
	endtry

	CHECK_EQUAL_VAR(ret, 0)
End
