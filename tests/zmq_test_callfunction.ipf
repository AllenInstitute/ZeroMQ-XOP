#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=zmq_test_callfunction

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

Function ComplainsWithInvalidJSON1()

	string   msg
	string   replyMessage
	variable errorValue

	msg          = ""
	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_JSON_OBJECT)
End

Function ComplainsWithInvalidJSON2()

	string   msg
	string   replyMessage
	variable errorValue

	msg          = "abcd"
	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_JSON_OBJECT)
End

Function ComplainsWithInvalidVersion1()

	string   msg
	string   replyMessage
	variable errorValue

	msg          = "{}"
	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_VERSION)
End

Function ComplainsWithInvalidVersion2()

	string   msg
	string   replyMessage
	variable errorValue

	msg          = "{\"version\" : 0}"
	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_VERSION)
End

Function ComplainsWithInvalidVersion3()

	string   msg
	string   replyMessage
	variable errorValue

	msg          = "{\"version\" : 2}"
	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_VERSION)
End

Function ComplainsWithInvalidVersion4()

	string   msg
	string   replyMessage
	variable errorValue

	msg          = "{\"version\" : 1.1}"
	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_VERSION)
End

Function ComplainsWithInvalidMessageID1()

	string   msg
	string   replyMessage
	variable errorValue

	msg          = "{\"version\" : 1, \"messageID\" : null }"
	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_MESSAGEID)
End

Function ComplainsWithInvalidMessageId2()

	string   msg
	string   replyMessage
	variable errorValue

	msg          = "{\"version\" : 1, \"messageID\" : \"\"}"
	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_MESSAGEID)
End

Function ComplainsWithInvalidMessageID3()

	string msg
	string messageID = ""
	string   replyMessage
	variable errorValue

	messageID = PadString(messageID, 256, 0x20)

	sprintf msg, "{\"version\" : 1, \"messageID\" : \"%s\"}", messageID
	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_MESSAGEID)
End

Function ComplainsWithInvalidOperation()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1,"            + \
	      "\"unknownOperation\" : \"blah\"}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_OPERATION)
End

Function ComplainsWithInvalidOp1()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1,"    + \
	      "\"CallFunction\" : 4711}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_OPERATION)
End

Function ComplainsWithInvalidOp2()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1,"        + \
	      "\"CallFunction\" : \"blah\"}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_OPERATION)
End

Function ComplainsWithInvalidOpFmt1()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "   + \
	      "\"CallFunction\" : {" + \
	      " \"notName\" : \"a\"}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_OPERATION_FORMAT)
End

Function ComplainsWithInvalidOpFmt2()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "   + \
	      "\"CallFunction\" : {" + \
	      " \"name\" : 1.5}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_OPERATION_FORMAT)
End

Function ComplainsWithInvalidOpFmt3()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                      + \
	      "\"CallFunction\" : {"                    + \
	      "\"name\"      : \"TestFunctionNoArgs\"," + \
	      "\"notParams\" : \"\"}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_OPERATION_FORMAT)
End

Function ComplainsWithInvalidOpFmt4()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                   + \
	      "\"CallFunction\" : {"                 + \
	      "\"name\"   : \"TestFunctionNoArgs\"," + \
	      "\"params\" : 1}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_PARAM_FORMAT)
End

Function ComplainsWithInvalidParamsObj()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "               + \
	      "\"CallFunction\" : {"             + \
	      "\"name\" : \"TestFunction1Arg\"," + \
	      "\"params\" : [1, { \"type\" : 1}]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_PARAM_FORMAT)
End

// procedures not compiled is not tested as I don't know how...

Function ComplainsWithWrongNumObjects()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                     + \
	      "\"CallFunction\" : {"                   + \
	      "\"name\" : 1, \"blah\" : 2, \"blub\" : 3}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_OPERATION_FORMAT)
End

Function ComplainsWithEmptyFunctionName()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "   + \
	      "\"CallFunction\" : {" + \
	      "\"name\" : \"\"}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_NON_EXISTING_FUNCTION)
End

Function ComplainsWithNonExistFunction()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                 + \
	      "\"CallFunction\" : {"               + \
	      "\"name\" : \"FUNCTION_I_DONT_EXIST\"}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_NON_EXISTING_FUNCTION)
End

Function ComplainsWithTooFewParameters()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                + \
	      "\"CallFunction\" : {"              + \
	      "\"name\" : \"TestFunction2Args\"," + \
	      "\"params\" : [1]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_TOO_FEW_FUNCTION_PARAMS)
End

Function ComplainsWithTooManyParameters()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "               + \
	      "\"CallFunction\" : {"             + \
	      "\"name\" : \"TestFunction1Arg\"," + \
	      "\"params\" : [1, 2]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_TOO_MANY_FUNCTION_PARAMS)
End

Function ComplainsWithInvalidFuncSig1()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                      + \
	      "\"CallFunction\" : {"                    + \
	      "\"name\" : \"TestFunctionInvalidSig1\"," + \
	      "\"params\" : [\"blah\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_SIG)
End

Function ComplainsWithInvalidFuncSig2()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                      + \
	      "\"CallFunction\" : {"                    + \
	      "\"name\" : \"TestFunctionInvalidSig2\"," + \
	      "\"params\" : [0]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_SIG)
End

Function ComplainsWithInvalidFuncSig3()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                      + \
	      "\"CallFunction\" : {"                    + \
	      "\"name\" : \"TestFunctionInvalidSig3\"," + \
	      "\"params\" : [0]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_SIG)
End

Function ComplainsWithInvalidFuncSig4()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                        + \
	      "\"CallFunction\" : {"                      + \
	      "\"name\" : \"TestFunctionWithIntParam1\"," + \
	      "\"params\" : [0]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_SIG)
End

Function ComplainsWithInvalidFuncSig5()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                        + \
	      "\"CallFunction\" : {"                      + \
	      "\"name\" : \"TestFunctionWithIntParam2\"," + \
	      "\"params\" : [0]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_SIG)
End

Function ComplainsWithInvalidFuncSig6()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                        + \
	      "\"CallFunction\" : {"                      + \
	      "\"name\" : \"TestFunctionWithIntParam3\"," + \
	      "\"params\" : [0]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_SIG)
End

Function ComplainsWithInvalidFuncRet2()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                   + \
	      "\"CallFunction\" : {"                 + \
	      "\"name\" : \"TestFunctionInvalidRet2\"}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_RET)
End

Function ComplainsWithInvalidFuncRet3()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\" : 1, "                   + \
	      "\"CallFunction\" : {"                 + \
	      "\"name\" : \"TestFunctionInvalidRet3\"}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_RET)
End

Function ComplainsWithGarbageInParams()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\"     : 1, "                   + \
	      "\"CallFunction\" : {"                     + \
	      "\"name\"         : \"TestFunction1Arg\"," + \
	      "\"params\"       : [\"1.a\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_PARAM_FORMAT)
End

Function ComplainsWithInternalFunction()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\"     : 1, "      + \
	      "\"CallFunction\" : {"        + \
	      "\"name\"         : \"cos\"," + \
	      "\"params\"       : [1]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_NON_EXISTING_FUNCTION)
End

Function ComplainsWithTooLongFuncName()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\"     : 1, "                                                                                                                                                                                                                                                                                                                                                                                                                                                          + \
	      "\"CallFunction\" : {"                                                                                                                                                                                                                                                                                                                                                                                                                                                            + \
	      "\"name\"         : \"TestFunctionLongFunctionNameFromIgorProEightTestFunctionLongFunctionNameFromIgorProEightTestFunctionLongFunctionNameFromIgorProEightTestFunctionLongFunctionNameFromIgorProEightTestFunctionLongFunctionNameFromIgorProEightTestFunctionLongFunctionNameFromIgorProEightTestFunctionLongFunctionNameFromIgorProEightTestFunctionLongFunctionNameFromIgorProEightTestFunctionLongFunctionNameFromIgorProEightTestFunctionLongFunctionNameFromIgorProEight\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)

	CHECK_EQUAL_VAR(errorValue, REQ_NON_EXISTING_FUNCTION)
End

Function WorksWithFuncNoArgs1()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\"     : 1, "                    + \
	      "\"CallFunction\" : {"                      + \
	      "\"name\"         : \"TestFunctionNoArgs\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)
End

Function WorksWithFuncNoArgs2()

	string   msg
	string   replyMessage
	variable errorValue

	msg = "{\"version\"     : 1, "                               + \
	      "\"CallFunction\" : {"                                 + \
	      "\"name\"         : \"TestFunctionOptionalStructArg\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)
End

Function WorksWithFunc1ArgAndOpt()

	string msg
	string replyMessage
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                         + \
	      "\"CallFunction\" : {"                           + \
	      "\"name\"         : \"TestFunction1ArgAndOpt\"," + \
	      " \"params\" : [1]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(resultVariable, 1)
End

Function WorksWithFunc2Vars()

	string   msg
	variable expected
	string replyMessage, resultString
	variable errorValue, resultVar

	msg = "{\"version\" : 1, "                + \
	      "\"CallFunction\" : {"              + \
	      "\"name\" : \"TestFunction2Args\"," + \
	      "\"params\" : [1, 2]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVar)
	expected = 1 + 2
	CHECK_EQUAL_VAR(resultVar, expected)
End

Function WorksWithFunc2Strings()

	string msg, expected
	string replyMessage, resultString
	variable errorValue

	msg = "{\"version\" : 1, "                      + \
	      "\"CallFunction\" : {"                    + \
	      "\"name\" : \"TestFunction2ArgsString\"," + \
	      "\"params\" : [\"1\", \"2\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, str = resultString)
	expected = "1_2"
	CHECK_EQUAL_STR(resultString, expected)
End

Function WorksWithFuncStrVarStr()

	string msg, expected
	string replyMessage, resultString
	variable errorValue

	msg = "{\"version\" : 1, "                    + \
	      "\"CallFunction\" : {"                  + \
	      "\"name\" : \"TestFunctionStrVarStr\"," + \
	      "\"params\" : [\"1\", 2, \"3\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, str = resultString)
	expected = "1_2_3"
	CHECK_EQUAL_STR(resultString, expected)
End

Function WorksWithFuncVarArgAsStr()

	string msg
	string replyMessage
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                   + \
	      "\"CallFunction\" : {"                     + \
	      "\"name\"         : \"TestFunction1Arg\"," + \
	      "\"params\"       : [\"1\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(resultVariable, 1)
End

Function ComplainsWithNonStringNaN()

	string msg
	string replyMessage
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                   + \
	      "\"CallFunction\" : {"                     + \
	      "\"name\"         : \"TestFunction1Arg\"," + \
	      "\"params\"       : [NaN]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_INVALID_JSON_OBJECT)
End

Function WorksWithFuncVarArgAsInfPlus1()

	string msg
	string replyMessage
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                   + \
	      "\"CallFunction\" : {"                     + \
	      "\"name\"         : \"TestFunction1Arg\"," + \
	      "\"params\"       : [\"+INF\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(resultVariable, Inf)
End

Function WorksWithFuncVarArgAsInfPlus2()

	string msg
	string replyMessage
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                   + \
	      "\"CallFunction\" : {"                     + \
	      "\"name\"         : \"TestFunction1Arg\"," + \
	      "\"params\"       : [\"+inf\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(resultVariable, Inf)
End

Function WorksWithFuncVarArgAsInfPlus3()

	string msg
	string replyMessage
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                   + \
	      "\"CallFunction\" : {"                     + \
	      "\"name\"         : \"TestFunction1Arg\"," + \
	      "\"params\"       : [\"inf\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(resultVariable, Inf)
End

Function WorksWithFuncVarArgAsInfMinus()

	string msg
	string replyMessage
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                   + \
	      "\"CallFunction\" : {"                     + \
	      "\"name\"         : \"TestFunction1Arg\"," + \
	      "\"params\"       : [\"-inf\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(resultVariable, -Inf)
End

Function WorksWithFuncVarArgAsNaN()

	string msg
	string replyMessage
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                   + \
	      "\"CallFunction\" : {"                     + \
	      "\"name\"         : \"TestFunction1Arg\"," + \
	      "\"params\"       : [\"naN\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(resultVariable, NaN)
End

Function WorksWithFuncVarArgAsVar()

	string msg
	string replyMessage
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                   + \
	      "\"CallFunction\" : {"                     + \
	      "\"name\"         : \"TestFunction1Arg\"," + \
	      "\"params\"       : [1]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(resultVariable, 1)
End

Function WorksWithFuncVarArgAsBoolean()

	string msg
	string replyMessage
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                   + \
	      "\"CallFunction\" : {"                     + \
	      "\"name\"         : \"TestFunction1Arg\"," + \
	      "\"params\"       : [true]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(resultVariable, 1)
End

Function WorksWithFuncVarArgAndFullPrec()

	string msg
	string replyMessage
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                   + \
	      "\"CallFunction\" : {"                     + \
	      "\"name\"         : \"TestFunction1Arg\"," + \
	      "\"params\"       : [1.23456789101112]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(resultVariable, 1.23456789101112)
End

Function WorksWithFuncStrArg1()

	string   msg
	string   replyMessage
	variable errorValue
	string resultString, expected

	msg = "{\"version\"     : 1, "                      + \
	      "\"CallFunction\" : {"                        + \
	      "\"name\"         : \"TestFunction1StrArg\"," + \
	      "\"params\"       : [\"hi\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, str = resultString)
	expected = "prefix__hi__suffix"
	CHECK_EQUAL_STR(resultString, expected)
End

Function WorksWithFuncNullWaveReturn()

	string                msg
	string                replyMessage
	variable              errorValue
	string                expected
	STRUCT WaveProperties s

	msg = "{\"version\"     : 1, "                            + \
	      "\"CallFunction\" : {"                              + \
	      "\"name\"         : \"TestFunctionReturnNullWave\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, wvProp = s)
	CHECK(!WaveExists(s.raw))
	CHECK(!WaveExists(s.dimensions))
	CHECK_EQUAL_VAR(numtype(s.modificationDate), 2)
End

Function WorksWithFuncReturnPermWave()

	string msg, replyMessage, expected
	variable              errorValue
	STRUCT WaveProperties s

	msg = "{\"version\"     : 1, "                            + \
	      "\"CallFunction\" : {"                              + \
	      "\"name\"         : \"TestFunctionReturnPermWave\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, wvProp = s)
	WAVE wv = TestFunctionReturnPermWave()
	CompareWaveWithSerialized(wv, s)
End

Function WorksWithFuncReturnFreeWave()

	string msg, replyMessage, expected
	variable              errorValue
	STRUCT WaveProperties s

	msg = "{\"version\"     : 1, "                            + \
	      "\"CallFunction\" : {"                              + \
	      "\"name\"         : \"TestFunctionReturnFreeWave\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, wvProp = s)
	WAVE wv = TestFunctionReturnFreeWave()
	CompareWaveWithSerialized(wv, s)
End

Function WorksWithFuncReturnWaveWave()

	string msg, replyMessage, expected
	variable              errorValue
	STRUCT WaveProperties s

	msg = "{\"version\"     : 1, "                            + \
	      "\"CallFunction\" : {"                              + \
	      "\"name\"         : \"TestFunctionReturnWaveWave\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, wvProp = s)
	WAVE wv = TestFunctionReturnWaveWave()
	CompareWaveWithSerialized(wv, s)
End

Function WorksWithFuncReturnDFWave()

	string msg, replyMessage, expected
	variable              errorValue
	STRUCT WaveProperties s

	msg = "{\"version\"     : 1, "                          + \
	      "\"CallFunction\" : {"                            + \
	      "\"name\"         : \"TestFunctionReturnDFWave\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, wvProp = s)
	WAVE wv = TestFunctionReturnDFWave()
	CompareWaveWithSerialized(wv, s)
End

Function ComplainsWithFuncAndIntParam1()

	string msg, replyMessage, expected
	variable              errorValue
	STRUCT WaveProperties s

	msg = "{\"version\"     : 1, "                            + \
	      "\"CallFunction\" : {"                              + \
	      "\"name\"         : \"TestFunctionWithIntParam1\"," + \
	      "\"params\" : [1]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_SIG)
End

Function ComplainsWithFuncAndIntParam2()

	string msg, replyMessage, expected
	variable              errorValue
	STRUCT WaveProperties s

	msg = "{\"version\"     : 1, "                            + \
	      "\"CallFunction\" : {"                              + \
	      "\"name\"         : \"TestFunctionWithIntParam2\"," + \
	      "\"params\" : [1]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_SIG)
End

Function ComplainsWithFuncAndIntParam3()

	string msg, replyMessage, expected
	variable              errorValue
	STRUCT WaveProperties s

	msg = "{\"version\"     : 1, "                            + \
	      "\"CallFunction\" : {"                              + \
	      "\"name\"         : \"TestFunctionWithIntParam3\"," + \
	      "\"params\" : [1]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_SIG)
End

Function ComplainsWithFuncAndWaveByRefParam4()

	string msg, replyMessage
	variable errorValue

	msg = "{\"version\"     : 1, "                         + \
	      "\"CallFunction\" : {"                           + \
	      "\"name\"         : \"TestFunctionPassByRef7\"," + \
	      "\"params\" : [1]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_UNSUPPORTED_FUNC_SIG)
End

// IP7 style "double" parameters are accepted
Function WorksWithFuncAndDoubleParam()

	string msg, replyMessage, expected
	variable              errorValue
	STRUCT WaveProperties s

	msg = "{\"version\"     : 1, "                              + \
	      "\"CallFunction\" : {"                                + \
	      "\"name\"         : \"TestFunctionWithDoubleParam\"," + \
	      "\"params\" : [1]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)
End

Function/S RunFlakyAbortChecks(string msg)

	string replyMessage
	variable errorValue, i

	for(i = 0; i < 100; i += 1)
		try
			replyMessage = zeromq_test_callfunction(msg)
		catch
			errorValue = ExtractErrorValue(replyMessage)

			if(errorValue == REQ_SUCCESS)
				continue
			endif

			CHECK_EQUAL_VAR(errorValue, REQ_FUNCTION_ABORTED)
			return replyMessage
		endtry
	endfor

	FAIL()
End

Function WorksWithFunctionsWhichAbort1()

	string msg, replyMessage
	variable pos

	msg = "{\"version\"     : 1, "                    + \
	      "\"CallFunction\" : {"                      + \
	      "\"name\"         : \"TestFunctionAbort1\"" + \
	      "}}"

	replyMessage = RunFlakyAbortChecks(msg)
	pos          = strsearch(replyMessage, "\"history\"", 0)
	CHECK(pos >= 0)

	pos = strsearch(replyMessage, "TestFunctionAbort1: abort message", 0)
	CHECK(pos >= 0)
End

Function WorksWithFunctionsWhichAbort2()

	string msg

	msg = "{\"version\"     : 1, "                    + \
	      "\"CallFunction\" : {"                      + \
	      "\"name\"         : \"TestFunctionAbort2\"" + \
	      "}}"

	RunFlakyAbortChecks(msg)
End

Function WorksWithFunctionsWhichAbort3AndHavePassByRefParams()

	string msg, replyMessage
	string expected, actual
	variable errorValue, resultVariable, i

	msg = "{\"version\"     : 1, "                         + \
	      "\"CallFunction\" : {"                           + \
	      "\"name\"         : \"TestFunctionPassByRef4\"," + \
	      "\"params\" : [123, \"nothing\"]}}"

	RunFlakyAbortChecks(msg)
End

Function WorksWithFunctionsAndPassByRef1()

	string msg, replyMessage
	variable expected, actual
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                         + \
	      "\"CallFunction\" : {"                           + \
	      "\"name\"         : \"TestFunctionPassByRef1\"," + \
	      "\"params\" : [1]}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	expected = 42
	CHECK_EQUAL_VAR(expected, resultVariable)

	Make/FREE/T/N=0 wv
	ExtractReturnValue(replyMessage, passByRefWave = wv)
	expected = 4711
	actual   = str2num(wv[0])
	CHECK_EQUAL_VAR(expected, actual)
End

Function WorksWithFunctionsAndPassByRef2()

	string msg, replyMessage
	string expected, actual
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                         + \
	      "\"CallFunction\" : {"                           + \
	      "\"name\"         : \"TestFunctionPassByRef2\"," + \
	      "\"params\" : [\"nothing\"]}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(42, resultVariable)

	Make/FREE/T/N=0 wv
	ExtractReturnValue(replyMessage, passByRefWave = wv)
	expected = "hi there"
	actual   = wv[0]
	CHECK_EQUAL_STR(expected, actual)
End

Function WorksWithFunctionsAndPassByRef3()

	string msg, replyMessage
	string expected, actual
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                         + \
	      "\"CallFunction\" : {"                           + \
	      "\"name\"         : \"TestFunctionPassByRef3\"," + \
	      "\"params\" : [123, \"nothing\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	print replyMessage

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(42, resultVariable)

	Make/FREE/T/N=0 wv
	ExtractReturnValue(replyMessage, passByRefWave = wv)
	expected = "nan"
	actual   = wv[0]
	CHECK_EQUAL_STR(expected, actual)

	expected = "hi there"
	actual   = wv[1]
	CHECK_EQUAL_STR(expected, actual)
End

Function WorksWithFunctionsAndPassByRef6()

	string msg, replyMessage
	string expected, actual
	variable errorValue, resultVariable

	msg = "{\"version\"     : 1, "                         + \
	      "\"CallFunction\" : {"                           + \
	      "\"name\"         : \"TestFunctionPassByRef6\"," + \
	      "\"params\" : []}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(42, resultVariable)
End

Function WorksWithReturningNullDFR()

	string msg, replyMessage
	string   expected
	variable errorValue
	string   resultString

	msg = "{\"version\"     : 1, "                           + \
	      "\"CallFunction\" : {"                             + \
	      "\"name\"         : \"TestFunctionReturnNullDFR\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, dfr = resultString)
	expected = "null"
	CHECK_EQUAL_STR(expected, resultString)
End

Function WorksWithReturningDFR()

	string msg, replyMessage
	string   expected
	variable errorValue
	string   resultString

	msg = "{\"version\"     : 1, "                       + \
	      "\"CallFunction\" : {"                         + \
	      "\"name\"         : \"TestFunctionReturnDFR\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, dfr = resultString)
	expected = "root:"
	CHECK_EQUAL_STR(expected, resultString)
End

Function WorksWithReturningFreeDFR()

	string msg, replyMessage
	string   expected
	variable errorValue
	string   resultString

	msg = "{\"version\"     : 1, "                           + \
	      "\"CallFunction\" : {"                             + \
	      "\"name\"         : \"TestFunctionReturnFreeDFR\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, dfr = resultString)
	expected = "free"
	CHECK_EQUAL_STR(expected, resultString)
End

Function WorksWithReturningDanglingDFR()

	string msg, replyMessage
	string   expected
	variable errorValue
	string   resultString

	msg = "{\"version\"     : 1, "                               + \
	      "\"CallFunction\" : {"                                 + \
	      "\"name\"         : \"TestFunctionReturnDanglingDFR\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, dfr = resultString)
	expected = "null"
	CHECK_EQUAL_STR(expected, resultString)
End

Function WorksWithPassingValidDFR1()

	string msg, replyMessage
	variable errorValue, resultVariable, expected

	msg = "{\"version\"     : 1, "                            + \
	      "\"CallFunction\" : {"                              + \
	      "\"name\"         : \"TestFunctionWithDFRParam1\"," + \
	      "\"params\": [\"root:\"]}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	expected = 123
	CHECK_EQUAL_VAR(expected, resultVariable)
End

Function WorksWithPassingValidDFR2()

	string msg, replyMessage
	variable errorValue
	string resultString, expected

	msg = "{\"version\"     : 1, "                            + \
	      "\"CallFunction\" : {"                              + \
	      "\"name\"         : \"TestFunctionWithDFRParam2\"," + \
	      "\"params\": [\"root:\"]}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, str = resultString)
	expected = "root:"
	CHECK_EQUAL_STR(expected, resultString)
End

Function WorksWithPassingValidDFR3()

	string msg, replyMessage
	variable errorValue
	string resultString, expected

	msg = "{\"version\"     : 1, "                            + \
	      "\"CallFunction\" : {"                              + \
	      "\"name\"         : \"TestFunctionWithDFRParam3\"," + \
	      "\"params\": [\"root:\"]}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, dfr = resultString)
	expected = "root:"
	CHECK_EQUAL_STR(expected, resultString)
End

Function WorksWithPassingMessageIDAndRep()
	string msg, replyMessage
	variable errorValue, resultVariable
	string expected, actual

	msg = "{\"version\"     : 1, "                    + \
	      " \"messageID\"   : \"4711\", "             + \
	      "\"CallFunction\" : {"                      + \
	      "\"name\"         : \"TestFunctionNoArgs\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	ExtractReturnValue(replyMessage, var = resultVariable)
	CHECK_EQUAL_VAR(resultVariable, NaN)

	expected = "4711"
	actual   = ExtractMessageID(replyMessage)
	CHECK_EQUAL_STR(expected, actual, case_sensitive = 1)
End

Function WorksWithLongNames()
	string msg, replyMessage
	variable errorValue, resultVariable
	string expected, actual

	msg = "{\"version\"     : 1, "                         + \
	      "\"CallFunction\" : {"                           + \
	      "\"name\"         : \"ReturnWaveWithLongNames\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	STRUCT WaveProperties s
	ExtractReturnValue(replyMessage, wvProp = s)
	WAVE wv = ReturnWaveWithLongNames()
	CompareWaveWithSerialized(wv, s)
End

Function WorksWithMultipleReturnValues1()
	string msg, replyMessage
	variable errorValue
	string expected, actual

	msg = "{\"version\"     : 1, "                                        + \
	      "\"CallFunction\" : {"                                          + \
	      "\"name\"         : \"TestFunctionMultipleReturnValuesValid1\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	Make/FREE/T/N=0 wv
	ExtractReturnValue(replyMessage, resultWave = wv)
	expected = "123"
	actual   = wv[0]
	CHECK_EQUAL_STR(expected, actual)

	expected = "Hi there!"
	actual   = wv[1]
	CHECK_EQUAL_STR(expected, actual)

	expected = "nan"
	actual   = wv[2]
	CHECK_EQUAL_STR(expected, actual)
End

Function WorksWithMultipleReturnValues2()
	string msg, replyMessage
	variable errorValue
	string expected, actual

	msg = "{\"version\"     : 1, "                                        + \
	      "\"CallFunction\" : {"                                          + \
	      "\"name\"         : \"TestFunctionMultipleReturnValuesValid2\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	Make/FREE/T/N=0 wv
	ExtractReturnValue(replyMessage, resultWave = wv)
	CHECK_EQUAL_VAR(DimSize(wv, 0), 1)

	STRUCT WaveProperties s
	ParseSerializedWave(replyMessage, s)
	Make/FREE/T refWave = num2str(p)
	CHECK_EQUAL_WAVES(refWave, s.raw)
	Make/FREE dims = {128, 0, 0, 0}
	CHECK_EQUAL_WAVES(dims, s.dimensions, tol = 0.1)
End

Function WorksWithMultipleReturnValues3()
	string msg, replyMessage
	variable errorValue
	string expected, actual, returnValue

	msg = "{\"version\"     : 1, "                                        + \
	      "\"CallFunction\" : {"                                          + \
	      "\"name\"         : \"TestFunctionMultipleReturnValuesValid3\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	Make/FREE/T/N=0 wv
	ExtractReturnValue(replyMessage, dfr = returnValue, resultWave = wv)
	CHECK_EQUAL_VAR(DimSize(wv, 0), 1)

	actual   = returnValue
	expected = "root:Packages:"
	CHECK_EQUAL_STR(actual, expected)
End

Function WorksWithMultipleReturnValues4()
	string msg, replyMessage
	variable errorValue
	string expected, actual, returnValue

	msg = "{\"version\"     : 1, "                                         + \
	      "\"CallFunction\" : {"                                           + \
	      "\"name\"         : \"TestFunctionMultipleReturnValuesValid4\"," + \
	      "\"params\": [ 4711, \"my string\"]}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	Make/FREE/T/N=0 wv
	ExtractReturnValue(replyMessage, resultWave = wv)
	CHECK_EQUAL_VAR(DimSize(wv, 0), 2)

	expected = "4734"
	actual   = wv[0]
	CHECK_EQUAL_STR(expected, actual)

	expected = "my string!!"
	actual   = wv[1]
	CHECK_EQUAL_STR(expected, actual)
End

Function WorksWithMultipleReturnValues5()
	string msg, replyMessage
	variable errorValue
	string expected, actual

	msg = "{\"version\"     : 1, "                                        + \
	      "\"CallFunction\" : {"                                          + \
	      "\"name\"         : \"TestFunctionMultipleReturnValuesValid5\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)

	errorValue = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	Make/FREE/T/N=0 wv
	ExtractReturnValue(replyMessage, resultWave = wv)
	CHECK_EQUAL_VAR(DimSize(wv, 0), 2)

	expected = "0" // different default value compared to standard return values which use NaN
	actual   = wv[0]
	CHECK_EQUAL_STR(expected, actual)

	expected = ""
	actual   = wv[1]
	CHECK_EQUAL_STR(expected, actual)
End

Function WorksWithMultipleReturnValues6()

	string   msg
	string   replyMessage
	variable errorValue
	string expected, actual

	msg = "{\"version\"     : 1, "                                         + \
	      "\"CallFunction\" : {"                                           + \
	      "\"name\"         : \"TestFunctionMultipleReturnValuesValid6\"," + \
	      "\"params\"       : [4711, \"my string\"]}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	Make/FREE/T/N=0 resultWave
	ExtractReturnValue(replyMessage, resultWave = resultWave)
	CHECK_EQUAL_VAR(DimSize(resultWave, 0), 2)

	expected = "4734"
	actual   = resultWave[0]
	CHECK_EQUAL_STR(expected, actual)

	expected = "my string!!"
	actual   = resultWave[1]
	CHECK_EQUAL_STR(expected, actual)

	Make/FREE/T/N=0 passByRefWave
	ExtractReturnValue(replyMessage, passByRefWave = passByRefWave)
	CHECK_EQUAL_VAR(DimSize(passByRefWave, 0), 1)

	expected = "dummy text"
	actual   = passByRefWave[0]
	CHECK_EQUAL_STR(expected, actual)
End
