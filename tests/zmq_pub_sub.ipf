#pragma TextEncoding = "UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_pub_sub

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

Function Init_IGNORE()

	variable ret

	zeromq_set(ZMQ_SET_FLAGS_DEBUG | ZMQ_SET_FLAGS_DEFAULT | ZMQ_SET_FLAGS_LOGGING | ZMQ_SET_FLAGS_NOBUSYWAITRECV)

	ret = zeromq_pub_bind("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_sub_connect("tcp://127.0.0.1:5555")
	CHECK_EQUAL_VAR(ret, 0)
	CHECK_EQUAL_VAR(GetListeningStatus_IGNORE(5555, TCP_V4), 1)
End

Function DoesNotHaveAMessageFilterByDefault()

	int ret, i
	string msg, filter

	Init_IGNORE()

	ret = zeromq_pub_send("", "hi there!")
	CHECK_EQUAL_VAR(ret, 0)

	for(i = 0; i < 100; i += 1)
		msg = zeromq_sub_recv(filter)
		CHECK_EMPTY_STR(msg)
		CHECK_EMPTY_STR(filter)
	endfor
End

Function ReceivesHeartbeatMessagesWithEverythingFilter()

	int ret, i, found
	string msg, expected, filter

	Init_IGNORE()

	expected = ZMQ_HEARTBEAT
	ret = zeromq_sub_add_filter("")
	CHECK_EQUAL_VAR(ret, 0)

	for(i = 0; i < 200; i += 1)
		msg = zeromq_sub_recv(filter)
		if(strlen(msg) > 0 || strlen(filter) > 0)
			CHECK_EQUAL_STR(filter, expected)
			CHECK_EMPTY_STR(msg)
			found += 1
			break
		endif
		Sleep/S 0.1
	endfor

	CHECK(found > 0)
End

Function ReceivesHeartbeatMessagesWithCorrectFilter()

	int ret, i, found
	string msg, expected, filter

	Init_IGNORE()

	ret = zeromq_sub_add_filter("heart")
	CHECK_EQUAL_VAR(ret, 0)

	for(i = 0; i < 200; i += 1)
		msg = zeromq_sub_recv(filter)
		if(strlen(msg) > 0 || strlen(filter) > 0)
			expected = ZMQ_HEARTBEAT
			CHECK_EQUAL_STR(filter, expected)
			CHECK_EMPTY_STR(msg)
			found += 1
			break
		endif
		Sleep/S 0.1
	endfor

	CHECK(found > 0)
End

Function CanOnlySubscribeOnceToFilter()

	int ret, err

	Init_IGNORE()

	// empty string subcribes to everything
	ret = zeromq_sub_add_filter("")
	CHECK_EQUAL_VAR(ret, 0)

	try
		ret = zeromq_sub_add_filter(""); AbortOnRTE
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_MESSAGE_FILTER_DUPLICATED)
		CHECK_EQUAL_VAR(ret, 0)
	endtry
End

Function CanOnlyRemoveExistingFilter()

	int ret, err

	Init_IGNORE()

	// empty string subcribes to everything
	ret = zeromq_sub_add_filter("")
	CHECK_EQUAL_VAR(ret, 0)

	try
		ret = zeromq_sub_remove_filter("a"); AbortOnRTE
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_MESSAGE_FILTER_MISSING)
		CHECK_EQUAL_VAR(ret, 0)
	endtry
End

Function WorksWithCustomMessageAndFilter()

	int ret, err, i, found
	string msg, expected, filter

	Init_IGNORE()

	ret = zeromq_sub_add_filter("hi")
	CHECK_EQUAL_VAR(ret, 0)

	for(i = 0; i < 200; i += 1)
		ret = zeromq_pub_send("hi", "world!")
		CHECK_EQUAL_VAR(ret, 0)

		msg = zeromq_sub_recv(filter)
		if(strlen(msg) > 0 || strlen(filter) > 0)
			expected = "hi"
			CHECK_EQUAL_STR(filter, expected)
			expected = "world!"
			CHECK_EQUAL_STR(msg, expected)
			found += 1
			break
		endif
		Sleep/S 0.1
	endfor

	CHECK(found > 0)
End

Function WorksWithCustomMessageAndMultipleFilters()

	int ret, err, i, foundHi, foundHeart
	string msg, expected, filter

	Init_IGNORE()

	ret = zeromq_sub_add_filter("hi")
	CHECK_EQUAL_VAR(ret, 0)

	ret = zeromq_sub_add_filter("heart")
	CHECK_EQUAL_VAR(ret, 0)

	for(i = 0; i < 200; i += 1)
		ret = zeromq_pub_send("hi", "world!")
		CHECK_EQUAL_VAR(ret, 0)

		msg = zeromq_sub_recv(filter)
		if(strlen(msg) > 0 || strlen(filter) > 0)
			if(!cmpstr(filter, "hi"))
				expected = "world!"
				CHECK_EQUAL_STR(msg, expected)
				foundHi += 1
			endif
			if(!cmpstr(filter, ZMQ_HEARTBEAT))
				CHECK_EMPTY_STR(msg)
				foundHeart += 1
			endif

			if(foundHi > 0 && foundHeart > 0)
				break
			endif
		endif
		Sleep/S 0.1
	endfor

	CHECK(foundHi > 0)
	CHECK(foundHeart > 0)
End

Function AddingAndRemovingFiltersWorks()

	int ret, err, i, j, foundHi, foundHeart
	string msg, expected, filter

	Init_IGNORE()

	ret = zeromq_sub_add_filter("hi")
	CHECK_EQUAL_VAR(ret, 0)

	for(i = 0; i < 200; i += 1)
		ret = zeromq_pub_send("hi", "world!")
		CHECK_EQUAL_VAR(ret, 0)

		msg = zeromq_sub_recv(filter)
		if(strlen(msg) > 0 || strlen(filter) > 0)
			expected = "hi"
			CHECK_EQUAL_STR(filter, expected)
			expected = "world!"
			CHECK_EQUAL_STR(msg, expected)
			foundHi += 1

			zeromq_sub_remove_filter("hi")
			// remove existing hi messages
			for(;;)
				msg = zeromq_sub_recv(filter)
				if(strlen(msg) > 0 || strlen(filter) > 0)
					expected = "hi"
					CHECK_EQUAL_STR(filter, expected)
					expected = "world!"
					CHECK_EQUAL_STR(msg, expected)
					continue
				endif
				break
			endfor

			zeromq_sub_add_filter("heartbeat")

			for(j = 0; j < 200; j += 1)
				msg = zeromq_sub_recv(filter)
				if(strlen(msg) > 0 || strlen(filter) > 0)
					expected = ZMQ_HEARTBEAT
					CHECK_EQUAL_STR(filter, expected)
					CHECK_EMPTY_STR(msg)
					foundHeart += 1
					break
				endif
				Sleep/S 0.1
			endfor

			break
		endif
		Sleep/S 0.1
	endfor

	CHECK(foundHi > 0)
	CHECK(foundHeart > 0)
End
