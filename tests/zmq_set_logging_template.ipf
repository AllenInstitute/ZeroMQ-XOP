#pragma TextEncoding="UTF-8"
#pragma rtGlobals=3
#pragma ModuleName=ZMQ_LOGGING

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

static Function/WAVE InvalidJSONTemplatesGen()

	Make/FREE/T/N=(5) templates

	templates[0] = "not-a-valid-json"

	SetDimLabel 0, 0, NotAValidJSON, templates

	templates[1] = "[]"

	SetDimLabel 0, 1, ValidJSONWithWrongTopLevelType, templates

	templates[2] = "{\"str\" : []}"

	SetDimLabel 0, 2, ValidJSONWithReservedObjectNames, templates

	templates[3] = "{\"json\" : [] }"

	SetDimLabel 0, 3, ValidJSONWithReservedObjectNames, templates

	templates[4] = "{\"direction\" : []}"

	SetDimLabel 0, 4, ValidJSONWithReservedObjectNames, templates

	return templates
End

static Function/S GetLogFilePath_IGNORE()

	string path

	path = SpecialDirPath("Igor Preferences", 0, 0, 1) + "Packages:"
	NewPath/Q/C/O myPath, path
	path += "ZeroMQ:"
	NewPath/Q/C/O myPath, path
	path += "Log.jsonl"

	return path
End

static Function/S ReadFile(string path)

	variable refNum
	string   contents

	Open/R/Z=1 refNum as path

	CHECK_EQUAL_VAR(V_Flag, 0)

	FStatus refNum

	contents = PadString("", V_logEOF, 0x0)
	FBinRead refnum, contents
	Close refnum

	return contents
End

static Function TEST_CASE_BEGIN_OVERRIDE(name)
	string name

	string path

	path = GetLogFilePath_IGNORE()

	DeleteFile/Z=1 path
	CHECK(V_Flag == 0 || V_Flag == -43)

	zeromq_stop()
	zeromq_set(ZMQ_SET_FLAGS_DEBUG | ZMQ_SET_FLAGS_DEFAULT | ZMQ_SET_FLAGS_LOGGING)
End

static Function TEST_CASE_END_OVERRIDE(name)
	string name

	DoXOPIdle

	zeromq_stop()
End

// UTF_TD_GENERATOR ZMQ_LOGGING#InvalidJSONTemplatesGen
Function InvalidJSONTemplates([string str])

	variable err
	string   path

	path = GetLogFilePath_IGNORE()
	GetFileFolderInfo/Q/Z=1 path
	CHECK_NEQ_VAR(V_flag, 0)

	try
		zeromq_set_logging_template(str); AbortOnRTE
		FAIL()
	catch
		err = GetRTError(1)
		CheckErrorMessage(err, ZMQ_INVALID_LOGGING_TEMPLATE)
	endtry
End

Function WorksInBothDirections()

	variable errorValue
	string path, expected, actual, msg, replyMessage, logfile

	path = GetLogFilePath_IGNORE()
	GetFileFolderInfo/Q/Z=1 path
	CHECK_NEQ_VAR(V_flag, 0)

	zeromq_set_logging_template("{\"blahh\" : \"blubb\"}")

	GetFileFolderInfo/Q/Z=1 path
	CHECK_EQUAL_VAR(V_flag, 0)

	actual   = ReadFile(path)
	expected = "{}\n"
	CHECK_EQUAL_STR(actual, expected)

	msg = "{\"version\"     : 1, "                    + \
	      "\"CallFunction\" : {"                      + \
	      "\"name\"         : \"TestFunctionNoArgs\"" + \
	      "}}"

	replyMessage = zeromq_test_callfunction(msg)
	errorValue   = ExtractErrorValue(replyMessage)
	CHECK_EQUAL_VAR(errorValue, REQ_SUCCESS)

	logfile = ReadFile(path)
	CHECK_EQUAL_VAR(ItemsInList(logfile, "\n"), 3)

	actual   = "{}"
	expected = StringFromList(0, logfile, "\n")

	actual   = "{\"blahh\":\"blubb\",\"direction\":\"Incoming\",\"json\":{\"CallFunction\":{\"name\":\"TestFunctionNoArgs\"},\"version\":1}}"
	expected = StringFromList(1, logfile, "\n")

	actual   = "{\"blahh\":\"blubb\",\"direction\":\"Outgoing\",\"json\":{\"errorCode\":{\"value\":0},\"result\":{\"type\":\"variable\",\"value\":\"nan\"}}}"
	expected = StringFromList(2, logfile, "\n")
End
