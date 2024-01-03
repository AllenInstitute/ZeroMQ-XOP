#pragma TextEncoding = "UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=ZMQ_LEAK_TESTING

/// Search memory leaks and try to ensure that these are not present.
///
/// Logic:
/// - Every test calls a function via zeromq_test_callfunction
/// - This XOP call needs to take care of releasing the allocated memory
/// - We allocate roughly NUM_BYTES_LEAK_TESTING * NUM_RUNS bytes (10MB with current values)
/// - We later check that the amount of used memory only increased by a smaller margin

Constant NUM_BYTES_LEAK_TESTING = 1e5
static Constant ADDITIONAL_MEMORY_USED = 8e6
static Constant NUM_RUNS = 100

// Memory allocation works differently on MacOSX so we only can test that on Windows.

#ifdef MACINTOSH

Function EmptyTest()
	PASS()
End

#else

static Function TEST_CASE_BEGIN_OVERRIDE(name)
	string name

	CHECK(	NUM_BYTES_LEAK_TESTING * NUM_RUNS > ADDITIONAL_MEMORY_USED)

	zeromq_stop()
	zeromq_set(ZMQ_SET_FLAGS_DEFAULT)

	printf "BEGIN: Used memory %W0PB\r", NumberByKey("USEDPHYSMEM", IgorInfo(0))
End

static Function TEST_CASE_END_OVERRIDE(name)
	string name

	printf "END: Used memory %W0PB\r", NumberByKey("USEDPHYSMEM", IgorInfo(0))

	DoXOPIdle

	zeromq_stop()
End

static Function/WAVE GenerateMessage()

	string contents

	Make/FREE/WAVE/T/N=(4) messages

	messages[0] = "{\"version\"     : 1, "                                 + \
	              "\"CallFunction\" : {"                                   + \
	              "\"name\"         : \"TestFunctionReturnLargeFreeWave\"" + \
	              "}}"

	SetDimLabel 0, 0, ReturnLargeFreeWave, messages

	messages[1] = "{\"version\"     : 1, "                                   + \
	              "\"CallFunction\" : {"                                     + \
	              "\"name\"         : \"TestFunctionReturnLargeDataFolder\"" + \
	              "}}"

	SetDimLabel 0, 1, ReturnLargeDataFolder, messages

	contents = PadString("", NUM_BYTES_LEAK_TESTING, 0x20)

	messages[2] = "{\"version\"     : 1, "                      + \
		          "\"CallFunction\" : {"                        + \
		          "\"name\"         : \"TestFunction1StrArg\"," + \
		          "\"params\"       : [\"" + contents + "\"]}}"

	SetDimLabel 0, 2, StringParameter, messages

	messages[3] = "{\"version\"     : 1, "                         + \
		          "\"CallFunction\" : {"                           + \
		          "\"name\"         : \"TestFunctionPassByRef5\"," + \
		          "\"params\" : [\"nothing\", 123]}}"

	SetDimLabel 0, 3, StringPassByRefParameter, messages

	return messages
End

// UTF_TD_GENERATOR ZMQ_LEAK_TESTING#GenerateMessage
Function DoesNotHaveMemoryLeaks([string msg])
	variable i, memBefore, memAfter

	string replyMessage

	memBefore = NumberByKey("USEDPHYSMEM", IgorInfo(0))

	for(i = 0; i < NUM_RUNS; i++)
		replyMessage = zeromq_test_callfunction(msg)
	endfor

	memAfter = NumberByKey("USEDPHYSMEM", IgorInfo(0))

	CHECK(memAfter < (memBefore + ADDITIONAL_MEMORY_USED))
End

#endif
