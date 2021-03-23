#pragma TextEncoding = "UTF-8"
#pragma rtGlobals=3		// Use modern global access method and strict wave access.
#pragma IgorVersion=8.0

#include "::procedures:ZeroMQ_Interop"

Constant TCP_V4 = 4
Constant TCP_V6 = 6

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

/// @brief Return the amount of free memory in GB
///
/// Due to memory fragmentation you can not assume that you can still create a wave
/// occupying as much space as returned.
Function GetFreeMemory()
	variable freeMem

#if defined(IGOR64)
	freeMem = NumberByKey("PHYSMEM", IgorInfo(0)) - NumberByKey("USEDPHYSMEM", IgorInfo(0))
#else
	freeMem = NumberByKey("FREEMEM", IgorInfo(0))
#endif

	return freeMem / 1024 / 1024 / 1024
End

Function CheckErrorMessage(returnedError, xopError)
	variable returnedError, xopError

	string errorMessage = GetErrMessage(returnedError)
	CHECK(strlen(errorMessage) > 0)

//	From http://www.igorexchange.com/node/7286:
//	I'm not sure, but I believe that, if you AND with 0xFFFF and add FIRST_XOP_ERR,
//	you will get your error code. This is an implementation detail and subject to change.
	CHECK_EQUAL_VAR((returnedError & 0xFFFF) + 10000, xopError)
End

/// @brief Check using netstat that a process listens on the given port
///
/// Adapted and inspired by http://www.igorexchange.com/node/1243
Function GetListeningStatus_IGNORE(port, tcpVersion)
	variable port, tcpVersion

	string tmpDir, symbDirPath, filename, cmd, fullPath, localhost
	string contents = ""
	variable refNum

#ifdef WINDOWS

	if(tcpVersion == 4)
		localhost = "0.0.0.0:0"
	elseif(tcpVersion == 6)
		localhost = "\[::\]:0"
	else
		FAIL()
	endif

	tmpDir = SpecialDirPath("Temporary", 0, 0, 0)

	// Make sure that the directory we just got is, in fact, a directory.
	GetFileFolderInfo/Q tmpDir
	AbortOnValue (V_Flag >= 0 && !V_isFolder), 3

	// Set an Igor symbolic path to the temporary directory.
	symbDirPath = UniqueName("tmpPath", 12, 0)
	NewPath/Q $(symbDirPath), tmpDir
	AbortOnValue (V_flag), 5

	sprintf filename "igor_port_listening_%s.txt", Hash(num2istr(DateTime), 1)

	// Add a path separator character to the end of the path, if necessary, and add on the file name.
	fullPath = ParseFilePath(2, tmpDir, ":", 0, 0) + filename

	// Convert the path into a windows path that uses "\" as the path separator.
	fullPath = ParseFilePath(5, fullPath, "\\", 0, 0)

	sprintf cmd, "cmd /C \"netstat -fAN | findStr %d | findStr %s > %s\"", port, localhost, fullPath
	ExecuteScriptText/B/W=2 cmd
	AbortOnValue (V_flag != 0), 7

	// Check that the temporary results file exists.
	GetFileFolderInfo/Z=1/Q/P=$(symbDirPath) filename
	AbortOnValue (V_flag != 0 || !(V_isFile)), 10

	contents = PadString(contents, V_logEOF, 0)

	// Get the results from the temporary file created by the batch file.
	Open/P=$(symbDirPath)/R/Z=1 refNum as filename
	KillPath/Z $(symbDirPath)
	AbortOnValue (V_flag != 0), 12

	FBinRead refNum, contents
	Close refNum

	return strlen(contents) > 0
#else
	sprintf cmd, "do shell script \"netstat -an\""

	ExecuteScriptText/Z cmd
	AbortOnValue (V_flag != 0), 7

	// we want to match the line
	// tcp4       0      0  127.0.0.1.5555         *.*                    LISTEN
	return GrepString(S_Value, ".*" + "\." + num2str(port) + ".*LISTEN")
#endif

End

Function TEST_CASE_BEGIN_OVERRIDE(name)
	string name

	zeromq_stop()
	zeromq_set(ZeroMQ_SET_FLAGS_DEBUG | ZeroMQ_SET_FLAGS_DEFAULT)
End

Function TEST_CASE_END_OVERRIDE(name)
	string name

	DoXOPIdle

	zeromq_stop()
End

// JSONSimple returns the following types in W_TokenType
// 0: number
// 1: object
// 2: array
// 3: string

Function ExtractErrorValue(replyMessage)
	string replyMessage

	string actual, expected
	variable errorCode

	CHECK_PROPER_STR(replyMessage)

	JSONSimple/Q/Z replyMessage

	WAVE/Z W_TokenType
	CHECK_WAVE(W_TokenType, NUMERIC_WAVE)
	REQUIRE(DimSize(W_TokenType, 0) > 3)
	CHECK_EQUAL_VAR(W_TokenType[0], 1)
	CHECK_EQUAL_VAR(W_TokenType[1], 3)
	CHECK_EQUAL_VAR(W_TokenType[2], 1)
	CHECK_EQUAL_VAR(W_TokenType[3], 3)

	WAVE/Z/T T_TokenText
	CHECK_WAVE(T_TokenText, TEXT_WAVE)

	actual   = T_TokenText[1]
	expected = "errorCode"
	CHECK_EQUAL_STR(actual, expected)

	FindValue/TXOP=4/TEXT="value" T_TokenText
	CHECK_NEQ_VAR(V_value,-1)

	errorCode = str2num(T_TokenText[V_value + 1])

	if(errorCode != REQ_SUCCESS)
		FindValue/TXOP=4/TEXT="msg" T_TokenText
		CHECK_NEQ_VAR(V_value,-1)
		CHECK(strlen(T_TokenText[V_Value + 1]) > 0)
	endif

	return errorCode
End

Function/S ExtractMessageID(replyMessage)
	string replyMessage

	string actual, expected
	string type = ""

	CHECK_PROPER_STR(replyMessage)

	JSONSimple/Q/Z replyMessage

	WAVE/Z/T T_TokenText
	CHECK_WAVE(T_TokenText, TEXT_WAVE)

	WAVE/Z W_TokenSize
	CHECK_WAVE(W_TokenSize, NUMERIC_WAVE)

	WAVE/Z W_TokenType
	CHECK_WAVE(W_TokenType, NUMERIC_WAVE)

	FindValue/TXOP=4/TEXT="messageID" T_TokenText
	CHECK_NEQ_VAR(V_value,-1)
	CHECK_EQUAL_VAR(W_TokenType[V_value + 1], 3)

	return T_TokenText[V_value + 1]
End

Function ExtractReturnValue(replyMessage, [var, str, dfr, wvProp, passByRefWave, resultWave])
	string replyMessage
	variable &var
	string &str
	string &dfr
	STRUCT WaveProperties &wvProp
	WAVE/T passByRefWave
	WAVE/T resultWave

	variable lastPassByRefRow, firstPassByRefRow
	variable i, idx, resultRow
	string actual, expected
	string type = ""

	CHECK_PROPER_STR(replyMessage)

	JSONSimple/Q/Z replyMessage

	WAVE/Z/T T_TokenText
	CHECK_WAVE(T_TokenText, TEXT_WAVE)

	WAVE/Z W_TokenSize
	CHECK_WAVE(W_TokenSize, NUMERIC_WAVE)

	WAVE/Z W_TokenType
	CHECK_WAVE(W_TokenType, NUMERIC_WAVE)

	if(!ParamIsDefault(var))
		type = "variable"
	elseif(!ParamIsDefault(str))
		type = "string"
	elseif(!ParamIsDefault(dfr))
		type = "dfref"
	elseif(!ParamIsDefault(wvProp))
		type = "wave"
	elseif(!ParamIsDefault(passByRefWave))
		// do nothing
	elseif(!ParamIsDefault(resultWave))
		// do nothing
	else
		FAIL()
	endif
	WAVE W_TokenType

	Duplicate/O T_TokenText, root:T_TokenText
	Duplicate/O W_TokenSize, root:W_TokenSize
	Duplicate/O W_TokenType, root:W_TokenType

	actual   = T_TokenText[1]
	expected = "errorCode"
	CHECK_EQUAL_STR(actual, expected)

	FindValue/TXOP=4/TEXT="result" T_TokenText
	resultRow = V_Value
	REQUIRE_NEQ_VAR(resultRow, -1)
	CHECK(W_TokenType[resultRow + 1] == 1 || W_TokenType[resultRow + 1] == 2)

	FindValue/TXOP=4/TEXT="type"/S=(resultRow) T_TokenText
	CHECK_NEQ_VAR(V_value,-1)

	if(strlen(type) > 0)
		actual   = T_TokenText[V_value + 1]
		expected = type
		CHECK_EQUAL_STR(actual, expected)
	endif

	if(!ParamIsDefault(var))
		var = str2num(T_TokenText[V_value + 3])
	elseif(!ParamIsDefault(str))
		str = T_TokenText[V_value + 3]
	elseif(!ParamIsDefault(dfr))
		dfr = T_TokenText[V_value + 3]
	elseif(!ParamIsDefault(wvProp))
		ParseSerializedWave(replyMessage, wvProp)
	elseif(!ParamIsDefault(passByRefWave))
		// do nothing
	elseif(!ParamIsDefault(resultWave))
		// do nothing
	else
		FAIL()
	endif

	if(!ParamIsDefault(passByRefWave))
		FindValue/TXOP=4/TEXT="passByReference" T_TokenText
	  CHECK_NEQ_VAR(V_value,-1)
	  Redimension/N=(W_TokenSize[V_value + 1]) passByRefWave
	  passByRefWave[] = T_TokenText[V_value + 2 + p]
	endif

	if(!ParamIsDefault(resultWave))
		lastPassByRefRow = DimSize(T_TokenText, 0) - 1

		Redimension/N=(W_TokenSize[resultRow + 1]) resultWave

		idx = 0
		for(i = resultRow; i <= lastPassByRefRow; i += 1)
			if(!cmpstr(T_TokenText[i], "value"))
				resultWave[idx] = T_TokenText[i + 1]
				idx++
			endif
		endfor
	endif
End

Function TestFunctionNoArgs()

End

Function TestFunction1ArgAndOpt(var1, [opt])
	variable var1
	string opt

	return 1
End

Function TestFunction1Arg(var1)
	variable var1

	return var1
End

Function/S TestFunction1StrArg(str1)
	string str1

	return "prefix__" + str1 + "__suffix"
End

Function TestFunction2Args(var1, var2)
	variable var1, var2

	return var1 + var2
End

Function/S TestFunction2ArgsString(str1, str2)
	string str1, str2

	return str1 + "_" + str2
End

Function/S TestFunctionStrVarStr(str1, var1, str2)
	string str1, str2
	variable var1

	return str1 + "_" + num2str(var1) + "_" + str2
End

Function TestFunctionOptionalStructArg([s])
	STRUCT WMBackgroundStruct &s
End

Function TestFunctionInvalidSig1(wv)
	WAVE wv
End

Function TestFunctionInvalidSig2(num)
	variable/C num
End

Function TestFunctionInvalidSig3(s)
	STRUCT WMBackgroundStruct &s
End

Function/C TestFunctionInvalidRet2()

	return cmplx(0, 1)
End

Function/WAVE TestFunctionReturnNullWave()
	return $""
End

Function/WAVE TestFunctionReturnPermWave()

	WAVE/Z/D data
	if(WAveExists(data))
		return data
	endif

	Make/O/D data = {1.5, 2.5}

	return data
End

Function/WAVE TestFunctionReturnFreeWave()

	Make/FREE/D data = {3, 4}

	return data
End

Function/WAVE TestFunctionReturnWaveWave()

	Make/FREE/D content = {3, 4}

	Make/FREE/WAVE data = {content}

	return data
End

Function/WAVE TestFunctionReturnDFWave()

	Make/FREE/DF data = {NewFreeDataFolder()}

	return data
End

Function/WAVE TestFunctionReturnLargeFreeWave()

	Make/N=(10^5)/R/FREE data = p

	return data
End

Function/DF TestFunctionReturnNullDFR()
	return $""
End

Function/DF TestFunctionReturnDFR()
	return $"root:"
End

Function/DF TestFunctionReturnFreeDFR()
	return NewFreeDataFolder()
End

Function/DF TestFunctionReturnDanglingDFR()

	DFREF old = GetDataFolderDFR()

	NewDataFolder/O/S test
	DFREF dfr = GetDataFolderDFR()

	SetDataFolder old
	KillDataFolder test

	return dfr
End

Function TestFunctionWithDFRParam1(dfr)
	DFREF dfr

	return 123
End

Function/S TestFunctionWithDFRParam2(dfr)
	DFREF dfr

	return GetDataFolder(1, dfr)
End

Function/DF TestFunctionWithDFRParam3(dfr)
	DFREF dfr

	return dfr
End

Function TestFunctionWithIntParam1(param)
	int param

	return param
End

Function TestFunctionWithIntParam2(param)
	int64 param

	return param
End

Function TestFunctionWithIntParam3(param)
	uint64 param

	return param
End

Function TestFunctionWithDoubleParam(param)
	double param

	return param
End

Function FunctionToCall()

	return 4711
End

Function TestFunctionAbort1()
	Abort
End

Function TestFunctionAbort2()
	AbortOnValue 1, 4711
End

Function TestFunctionPassByRef1(var)
	variable& var

	var = 4711

	return 42
End

Function TestFunctionPassByRef2(str)
	string& str

	str = "hi there"

	return 42
End

Function TestFunctionPassByRef3(var, str)
	variable& var
	string& str

	var = NaN
	str = "hi there"

	return 42
End

Function TestFunctionPassByRef4(var, str)
	variable& var
	string& str

	Abort

	return 42
End

Function TestFunctionPassByRef5(str, var)
	string& str
	variable& var

	var = 10e5
	str = ""
	str = PadString(str, var, 0x20)

	return 42
End

Function TestFunctionPassByRef6([s])
	STRUCT WMBackgroundStruct &s

	return 42
End
Function TestFunctionPassByRef7(WAVE& wv)

	Make/FREE/D freeWave = {4711}
	WAVE wv = freeWave

	return 42
End


Function/WAVE ReturnWaveWithLongNames()

	Make/O/N=1 AVeryLongNameOnlyAllowedWithIgorProEight
	WAVE wv = AVeryLongNameOnlyAllowedWithIgorProEight
	SetDimLabel 0, 0, AVeryLongLabelOnlyAllowedWithIgorProEight, wv

	return wv
End

Structure WaveProperties
	WAVE/T raw
	WAVE dimensions
	string type
	variable modificationDate
EndStructure

Function FindLastEntry(WAVE/T wv, string entry)

	variable index = -1

	// @todo ip9-only: use /R from FindValue
	for(;;)
		FindValue/S=(index + 1)/TXOP=4/TEXT=entry wv

		if(V_Value == -1)
			return index
		endif

		index = V_Value
	endfor
End

Function ParseSerializedWave(replyMessage, s)
	string& replyMessage
	STRUCT WaveProperties &s

	variable numTokens, start, index
	string expected, actual, typeLines, typeLine, type, dimLine, size0, size1, size2, size3

	CHECK_PROPER_STR(replyMessage)

	JSONSimple/Q/Z replyMessage

	WAVE/Z/T T_TokenText
	CHECK_WAVE(T_TokenText, TEXT_WAVE)

	WAVE/Z W_TokenSize
	REQUIRE(WaveExists(W_TokenSize))

	// "type": "NT_FP32"
	typeLines = GrepList(replyMessage, "\"type\": \".*_.*\"", 0, "\n")
	if(strlen(typeLines) > 0)
		// we want to use the last value
		typeLine = StringFromList(ItemsInList(typeLines, "\n") - 1, typeLines, "\n")
		SplitString/E="[[:space:]]\"type\": \"(.*)\"" typeLine, type
		CHECK_EQUAL_VAR(V_Flag, 1)
		s.type = type
	endif

	index = FindLastEntry(T_TokenText, "modification")

	if(index != -1)
		s.modificationDate = str2num(T_TokenText[index + 1])
	else
		s.modificationDate = NaN
	endif

	index = FindLastEntry(T_TokenText, "dimension")
	if(index != -1)

		dimLine = ReplaceString(" ", trimString(T_TokenText[index + 1], 2), "")
		string/g root:str = dimLine

		SplitString/E="\"size\":\[([[:digit:]]*),?([[:digit:]]*),?([[:digit:]]*),?([[:digit:]]*),?\]" dimLine, size0, size1, size2, size3
		CHECK(V_Flag >= 1)
		Make/N=(4)/I/FREE dimensions = {str2num(size0), str2num(size1), str2num(size2), str2num(size3)}
		dimensions[] = dimensions[p] == -1 ? 0 : dimensions[p]
		WAVE s.dimensions = dimensions
	endif

	FindValue/TXOP=4/TEXT="real" T_TokenText
	if(V_Value != -1)
		numTokens = W_TokenSize[V_Value + 1]

		Make/N=(2 * numTokens)/T/FREE raw
		raw[0, numTokens - 1] = T_TokenText[V_Value + 2 + p]

		FindValue/TXOP=4/TEXT="imag" T_TokenText
		CHECK_NEQ_VAR(V_value, -1)
		raw[numTokens, *] = T_TokenText[V_Value + 2 + p - numTokens]
	else
		FindValue/TXOP=4/TEXT="raw" T_TokenText
		if(V_Value == -1) // null wave?
			FindValue/TXOP=4/TEXT="wave" T_TokenText
			CHECK_NEQ_VAR(V_value, -1)
			expected = "null"
			actual   = T_TokenText[V_Value + 2]
			CHECK_EQUAL_STR(expected, actual)
			WAVE/Z/T raw = $""
		else
			CHECK_NEQ_VAR(V_value, -1)
			CHECK_NEQ_VAR(V_value, -1)
			numTokens = W_TokenSize[V_Value + 1]

			Make/N=(numTokens)/T/FREE raw = T_TokenText[V_Value + 2 + p]
		endif
	endif

	WAVE/T/Z s.raw = raw

	print s
	print s.raw
	print s.dimensions
End

Function/S GetWaveTypeString(wv)
	WAVE wv

	string result   = ""
	string modifier = ""

	variable type = WaveType(wv)

	if(type & COMPLEX_WAVE)
		type = type & ~COMPLEX_WAVE
		modifier += " | NT_CMPLX"
	endif

	if(type & UNSIGNED_WAVE)
		type = type & ~UNSIGNED_WAVE
		modifier += " | NT_UNSIGNED"
	endif

	switch(type)
		case FLOAT_WAVE:
			result = "NT_FP32"
			break
		case DOUBLE_WAVE:
			result = "NT_FP64"
			break
		case INT8_WAVE:
			result = "NT_I8"
			break
		case INT16_WAVE:
			result = "NT_I16"
			break
		case INT32_WAVE:
			result = "NT_I32"
			break
		case INT64_WAVE:
			result = "NT_I64"
			break
		case 0:
			switch(WaveType(wv, 1))
				case 2:
					result = "TEXT_WAVE_TYPE"
					break
				case 3:
					result = "DATAFOLDER_TYPE"
					break
				case 4:
					result = "WAVE_TYPE"
					break
				default:
					FAIL()
					break
			endswitch
			break
		default:
			FAIL()
			break
	endswitch

	return result + modifier
End

Function CompareWaveWithSerialized(wv, s)
	WAVE/Z wv
	STRUCT WaveProperties& s

	string expectedType, actualType
	variable numPoints, type

	REQUIRE(WaveExists(wv))
	REQUIRE(WaveExists(s.dimensions))
	REQUIRE(WaveExists(s.raw))

	// dimensions
	Make/FREE/N=(4)/I dims = DimSize(wv, p)
	CHECK_EQUAL_WAVES(dims, s.dimensions)

	if(s.modificationDate == 0)
		CHECK_EQUAL_VAR(ModDate(wv), s.modificationDate)
	else
		CHECK_EQUAL_VAR(ModDate(wv) - date2secs(1970, 1, 1), s.modificationDate)
	endif

	// type
	type = WaveType(wv)
	expectedType = GetWaveTypeString(wv)
	actualType   = s.type
	CHECK_EQUAL_STR(expectedType, actualType)

	numPoints = numpnts(s.raw)

	// content
	if(sum(dims) == 0)
		CHECK_EQUAL_VAR(numpnts(s.raw), 0)
	else
		if(!type) // non-numeric wave
			switch(WaveType(wv, 1))
				case 1: // numeric
					FAIL()
				case 2: // text
					Make/FREE/N=(numPoints)/T convWaveText
					// work around JSONSimple bug
					convWaveText[] = ReplaceString("\\\"", s.raw[p], "\"")
					Redimension/N=(dims[0], dims[1], dims[2], dims[3]) convWaveText
					CHECK_EQUAL_WAVES(wv, convWaveText, mode=WAVE_DATA)
					break
				case 3: // dfref wave
				case 4: // wave wave
					// no additional checks
					break
				default:
					FAIL()
			endswitch

		elseif(type & COMPLEX_WAVE)
			Make/FREE/N=(numPoints/2)/Y=(type)/C convWaveComplex
			convWaveComplex[] = cmplx(str2num(s.raw[p]), str2num(s.raw[numPoints / 2 + p]))
			Redimension/N=(dims[0], dims[1], dims[2], dims[3]) convWaveComplex
			CHECK_EQUAL_WAVES(wv, convWaveComplex, mode=WAVE_DATA)
		else
			Make/FREE/N=(numPoints)/Y=(type) convWave
			convWave[] = str2num(s.raw[p])
			Redimension/N=(dims[0], dims[1], dims[2], dims[3]) convWave
			// workaround IP9 bug when converting text containing NaN to numbers
			CHECK_EQUAL_WAVES(wv, convWave, mode=WAVE_DATA, tol = 1e-15)
		endif
	endif
End

/// @brief Exhaust all memory so that only `amountOfFreeMemoryLeft` [GB] is left
///
/// Unwise use of this function can break Igor!
Function ExhaustMemory(amountOfFreeMemoryLeft)
	variable amountOfFreeMemoryLeft

	variable i, expo=10, err
	string str

	for(i = expo; i >= 0;)
		err = GetRTError(1)
		str = UniqueName("base", 1, 0)
		Make/D/N=(10^expo) $str; err = GetRTError(1)

		if(err != 0)
			expo -= 1
		endif

		if(GetFreeMemory() < amountOfFreeMemoryLeft)
			break
		endif
	endfor

	printf "Free Memory: %gGB\r", GetFreeMemory()
End

Function/WAVE TestFunctionReturnExistingWave()

	WAVE/SDFR=root: bigWave

	return bigWave
End

// define MEMORY_LEAK_TESTING for testing against memory leaks
// these tests tend to be flaky, so they are not enabled by default

// Entry point for UTF
Function run()
	return RunWithOpts()
End

// Examples:
// - RunWithOpts()
// - RunWithOpts(testsuite = "zmq_set.ipf")
// - RunWithOpts(testcase = "StopsBinds")
Function RunWithOpts([string testcase, string testsuite, variable allowdebug])
	variable debugMode
	string list = ""
	string name = "ZeroMQ-XOP"

	// speeds up testing to start with a fresh copy
	KillWindow/Z HistoryCarbonCopy
	DisableDebugOutput()

	if(ParamIsDefault(allowdebug))
		debugMode = 0
	else
		debugMode = IUTF_DEBUG_FAILED_ASSERTION | IUTF_DEBUG_ENABLE | IUTF_DEBUG_ON_ERROR | IUTF_DEBUG_NVAR_SVAR_WAVE
	endif

	if(ParamIsDefault(testcase))
		testcase = ""
	endif

	// sorted list
	list = AddListItem("zmq_bind.ipf", list, ";", inf)
	list = AddListItem("zmq_connect.ipf", list, ";", inf)
	list = AddListItem("zmq_set.ipf", list, ";", inf)
	list = AddListItem("zmq_start_handler.ipf", list, ";", inf)
	list = AddListItem("zmq_stop.ipf", list, ";", inf)
	list = AddListItem("zmq_stop_handler.ipf", list, ";", inf)
	list = AddListItem("zmq_test_callfunction.ipf", list, ";", inf)
	list = AddListItem("zmq_test_interop.ipf", list, ";", inf)
	list = AddListItem("zmq_test_serializeWave.ipf", list, ";", inf)

	if(ParamIsDefault(testsuite))
		testsuite = list
	else
		// do nothing
	endif

	if(strlen(testcase) == 0)
		RunTest(testsuite, name = name, enableJU = 1, debugMode= debugMode)
	else
		RunTest(testsuite, name = name, enableJU = 1, debugMode= debugMode, testcase = testcase)
	endif
End
