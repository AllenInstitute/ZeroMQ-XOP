CMAKE_MINIMUM_REQUIRED(VERSION 3.15 FATAL_ERROR)
cmake_policy(SET CMP0091 NEW)

# This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

if(APPLE)
  # IP8 supports MacOSX 10.10 or higher
  # https://www.wavemetrics.com/products/igorpro
  SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "Minimum MacOSX version")
endif()

PROJECT(ZeroMQ)

OPTION(COVERAGE "Enable coverage instrumentation" OFF)
OPTION(SANITIZER "Enable sanitizer instrumentation" OFF)
OPTION(MSVC_RUNTIME_DYNAMIC "Link dynamically against the MSVC runtime library" OFF)
OPTION(WARNINGS_AS_ERRORS "Error out on compiler warnings" OFF)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Debug")
  MESSAGE(STATUS "CMAKE_BUILD_TYPE is not set, setting to CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}.")
ENDIF()

SET(XOP_MINIMUM_IGORVERSION 800)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/CreateLaunchers/cmake")
include(CreateLaunchers)

IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
  SET(bitness "")
  SET(bitnessWithDash "")
  SET(bitnessLibFolder "x86")

  IF(MSVC)
    SET(igorPath "C:/Program Files/WaveMetrics/Igor Pro 9 Folder/IgorBinaries_Win32/Igor.exe")
  ELSEIF(APPLE)
    MESSAGE(FATAL_ERROR "No 32bit support on MacOSX.")
  ENDIF()
ELSEIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(bitness "64")
  SET(bitnessWithDash "-64")
  SET(bitnessLibFolder "x64")

  IF(MSVC)
    SET(igorPath "C:/Program Files/WaveMetrics/Igor Pro 9 Folder/IgorBinaries_x64/Igor64.exe")
  ELSEIF(APPLE)
    SET(igorPath "/Applications/Igor\ Pro\ 9\ Folder/Igor64.app/Contents/MacOS/Igor64")
  ENDIF()
ENDIF()

SET(libname "${PROJECT_NAME}${bitnessWithDash}")

IF(APPLE)
  # use RPATH
  cmake_policy(SET CMP0042 NEW)
  SET(RESOURCES "${PROJECT_NAME}.r")
  SET(installFolder "${CMAKE_SOURCE_DIR}/../output/mac/xop/${CMAKE_BUILD_TYPE}")
  SET(installFolderLibZMQ "${CMAKE_SOURCE_DIR}/../output/mac/libzmq/${CMAKE_BUILD_TYPE}")
ELSEIF(WIN32)
  SET(RESOURCES "${PROJECT_NAME}.rc")
  SET(installFolder "${CMAKE_SOURCE_DIR}/../output/win/${bitnessLibFolder}/xop/${CMAKE_BUILD_TYPE}")
  SET(installFolderLibZMQ "${CMAKE_SOURCE_DIR}/../output/win/${bitnessLibFolder}/libzmq/${CMAKE_BUILD_TYPE}")
ENDIF()

# CPP files used for coverage analysis
SET(COVERAGE_SOURCES
  CallFunctionOperation.cpp
  CallFunctionParameterHandler.cpp
  ConcurrentXOPNotice.cpp
  CustomExceptions.cpp
  GlobalData.cpp
  HeartbeatPublisher.cpp
  HelperFunctions.cpp
  HistoryGrabber.cpp
  Logging.cpp
  MessageHandler.cpp
  RequestInterface.cpp
  RequestInterfaceException.cpp
  SerializeWave.cpp
  ZeroMQ.cpp
  zeromq_client_connect.cpp
  zeromq_client_recv.cpp
  zeromq_client_send.cpp
  zeromq_handler_start.cpp
  zeromq_handler_stop.cpp
  zeromq_pub_bind.cpp
  zeromq_pub_send.cpp
  zeromq_server_bind.cpp
  zeromq_server_recv.cpp
  zeromq_server_send.cpp
  zeromq_set.cpp
  zeromq_set_logging_template.cpp
  zeromq_stop.cpp
  zeromq_sub_add_filter.cpp
  zeromq_sub_connect.cpp
  zeromq_sub_recv.cpp
  zeromq_sub_remove_filter.cpp
  zeromq_test_callfunction.cpp
  zeromq_test_serializeWave.cpp
)

# CPP files used for clang-format
SET(SOURCES
  ${COVERAGE_SOURCES}
  functions.cpp
)

# CPP files *not* used for clang-format
SET(SOURCES_EXT
  git_version.cpp
  cmake_config.h
)

SET_SOURCE_FILES_PROPERTIES(git_version.cpp PROPERTIES GENERATED TRUE)
SET_SOURCE_FILES_PROPERTIES(cmake_config.h PROPERTIES GENERATED TRUE)

SET(HEADERS
  CallFunctionOperation.h
  CallFunctionParameterHandler.h
  ConcurrentQueue.h
  ConcurrentXOPNotice.h
  CustomExceptions.h
  Errors.h
  functions.h
  GlobalData.h
  HeartbeatPublisher.h
  HelperFunctions.h
  HistoryGrabber.h
  IgorTypeUnion.h
  Logging.h
  MessageHandler.h
  RequestInterface.h
  RequestInterfaceException.h
  resource.h
  SerializeWave.h
  SocketWithMutex.h
  ZeroMQ.h
  git_version.h
)

SOURCE_GROUP("Headers" FILES ${HEADERS})

# Remove that hack once we compile the XOPToolkit with cmake

SET(XOPTOOLKIT_SOURCES
 ${CMAKE_SOURCE_DIR}/../XOPSupport/IgorErrors.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/IgorXOP.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/NamedFIFO.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/WMTypes.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOP.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPContainers.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPContainers.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPDataFolderAccess.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPDialogsMac.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPDialogsWin.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPFileNavigationMac.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPFileNavigationWin.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPFiles.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPFilesMac.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPFilesWin.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPMemory.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPMenus.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPNumericConversion.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPResources.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPStandardHeaders.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPSupport.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPSupport.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPSupportMac.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPSupportMac.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPSupportWin.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPSupportWin.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPTextUtilityWindows.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPWaveAccess.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPWindows.c
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPWindows.h
 ${CMAKE_SOURCE_DIR}/../XOPSupport/XOPWinMacSupport.h
)

FOREACH(it IN LISTS ${XOPTOOLKIT_SOURCES})
  SET_SOURCE_FILES_PROPERTIES(${it} PROPERTIES HEADER_FILE_ONLY TRUE)
ENDFOREACH()

ADD_CUSTOM_TARGET("XOP_Toolkit" SOURCES ${XOPTOOLKIT_SOURCES}
                                COMMAND "echo"
                                COMMENT "Fake target for convenience. This does nothing.")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../XOPSupport)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${installFolderLibZMQ}/include)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

IF(APPLE)

  # heavily inspired by https://github.com/Kitware/CMake/blob/master/Tests/CFBundleTest/CMakeLists.txt

  SET(MISC
    InfoPlist.strings
    ${CMAKE_BINARY_DIR}/${libname}.rsrc)

  ADD_EXECUTABLE(${libname} MACOSX_BUNDLE ${SOURCES} ${SOURCES_EXT} ${HEADERS} ${RESOURCES} ${MISC})

  SET_TARGET_PROPERTIES(${libname} PROPERTIES PREFIX "")
  SET_TARGET_PROPERTIES(${libname} PROPERTIES BUNDLE_EXTENSION "xop")
  SET_TARGET_PROPERTIES(${libname} PROPERTIES ENABLE_EXPORTS true)
  SET_TARGET_PROPERTIES(${libname} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "${CMAKE_SOURCE_DIR}/Exports.exp")
  SET_TARGET_PROPERTIES(${libname} PROPERTIES XCODE_ATTRIBUTE_MACH_O_TYPE mh_bundle)

  SET(sysroot)
  IF(CMAKE_OSX_SYSROOT)
    SET(sysroot -isysroot ${CMAKE_OSX_SYSROOT})
  ENDIF()

  SET_TARGET_PROPERTIES(${libname} PROPERTIES CXX_STANDARD 14)

  FIND_LIBRARY(CARBON_LIBRARY Carbon)
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  FIND_LIBRARY(AUDIO_UNIT AudioUnit)
  FIND_LIBRARY(AUDIO_TOOLBOX AudioToolbox)
  MARK_AS_ADVANCED(CARBON_LIBRARY AUDIO_TOOLBOX AUDIO_UNIT COCOA_LIBRARY)
  SET(EXTRA_LIBS ${CARBON_LIBRARY} ${COCOA_LIBRARY} ${AUDIO_TOOLBOX} ${AUDIO_UNIT})

  FIND_PROGRAM(APPLE_RESOURCE Rez /Developer/Tools)
  IF(NOT APPLE_RESOURCE)
    MESSAGE(FATAL_ERROR "Could not find Rez to build resources from .r file.")
  ENDIF()

  ADD_CUSTOM_COMMAND(TARGET ${libname}
    PRE_BUILD
    COMMAND ${APPLE_RESOURCE}
    ${sysroot}
    -i ${CMAKE_SOURCE_DIR}/../XOPSupport
    -F ${CARBON_LIBRARY}
    -useDF
    ${RESOURCES}
    -o ${CMAKE_BINARY_DIR}/${libname}.rsrc
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

  ADD_CUSTOM_COMMAND(TARGET ${libname}
    POST_BUILD
    COMMAND zip
    -r ${libname}.zip
    ${CMAKE_CFG_INTDIR}/${libname}.xop
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  SET_SOURCE_FILES_PROPERTIES(
      ${CMAKE_BINARY_DIR}/${libname}.rsrc
      PROPERTIES GENERATED 1)

  SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_BINARY_DIR}/${libname}.rsrc
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

  SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_SOURCE_DIR}/InfoPlist.strings
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/English.lproj")

  TARGET_LINK_LIBRARIES(${libname} PRIVATE ${EXTRA_LIBS}
                         ${CMAKE_SOURCE_DIR}/../XOPSupport/Xcode/libXOPSupport${bitness}.a
                         ${installFolderLibZMQ}/lib/libzmq.a)

  TARGET_COMPILE_OPTIONS(${libname} PRIVATE -Weverything $<$<BOOL:${WARNINGS_AS_ERRORS}>:-Werror> -Wno-global-constructors -Wno-padded
                         -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic -Wno-c++98-compat
                         -Wno-reserved-id-macro -Wno-deprecated -Wno-parentheses -Wno-unused-function
                         # ignore __VA_ARGS__ problem with no elements, see https://stackoverflow.com/a/11172679
                         -Wno-gnu-zero-variadic-macro-arguments
                         # The following warning exclusion is a workaround for a warning popping up in the external fmt libary
                         -Wno-undef)
  TARGET_COMPILE_DEFINITIONS(${libname} PRIVATE TARGET_OS_MAC)
  TARGET_COMPILE_DEFINITIONS(${libname} PRIVATE $<$<CONFIG:DEBUG>:_DEBUG>)

  IF(${SANITIZER})
    MESSAGE(STATUS "Building with sanitizer support.")
    TARGET_COMPILE_OPTIONS(${libname} PRIVATE $<$<CONFIG:DEBUG>:-g -O2
                           -fsanitize=address -fsanitize=undefined -fsanitize=integer
                           -fsanitize=nullability -fno-omit-frame-pointer -fno-sanitize-recover=all
                           -fsanitize-recover=unsigned-integer-overflow>)
    TARGET_LINK_OPTIONS(${libname} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=address
                        -fsanitize=undefined -fsanitize=integer -fsanitize=nullability
                        -fno-omit-frame-pointer -fno-sanitize-recover=all
                        -fsanitize-recover=unsigned-integer-overflow>)
  ENDIF()

  SET(LLVM_BREW "/usr/local/opt/llvm/bin")

  IF(${COVERAGE})
    MESSAGE(STATUS "Building with coverage instrumentation")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-instr-generate")

    SET(coveragePath "${CMAKE_BINARY_DIR}/coverage")

    FIND_PROGRAM(LLVM_PROFDATA NAMES llvm-profdata PATHS ${LLVM_BREW} REQUIRED)

    ADD_CUSTOM_TARGET(coverage-merge
                      COMMAND ${LLVM_PROFDATA} merge -output=code.profdata default.profraw
                      WORKING_DIRECTORY ${coveragePath}
                      COMMENT "Generating coverage profraw files" VERBATIM)

    FIND_PROGRAM(LLVM_COV NAMES llvm-cov PATHS ${LLVM_BREW} REQUIRED)
    FIND_PROGRAM(CXX_DEMANGLER NAMES llvm-cxxfilt c++filt PATHS ${LLVM_BREW} REQUIRED)

    ADD_CUSTOM_TARGET(coverage-show
                      COMMAND ${LLVM_COV} show
                      ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${libname}.xop/Contents/MacOS/${libname}
                      -instr-profile=${coveragePath}/code.profdata
                      -show-instantiation-summary
                      ${COVERAGE_SOURCES}
                      -path-equivalence -use-color --format html
                      -Xdemangler=${CXX_DEMANGLER}
                      --output-dir=${coveragePath}
                      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                      COMMENT "Generating coverage HTML" VERBATIM)

    ADD_CUSTOM_TARGET(coverage-report
                      COMMAND ${LLVM_COV} report
                      ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${libname}.xop/Contents/MacOS/${libname}
                      -instr-profile=${coveragePath}/code.profdata
                      ${COVERAGE_SOURCES}
                      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                      COMMENT "Generating coverage summary" VERBATIM)

    ADD_CUSTOM_TARGET(coverage)

    # ORDER: coverage -> coverage-merge -> coverage-show -> coverage-report
    ADD_DEPENDENCIES(coverage coverage-report)
    ADD_DEPENDENCIES(coverage-report coverage-show)
    ADD_DEPENDENCIES(coverage-show coverage-merge)
  ENDIF()
ELSEIF(WIN32)

  ADD_LIBRARY(${libname} SHARED ${SOURCES} ${SOURCES_EXT} ${HEADERS} ${RESOURCES})

  SET_TARGET_PROPERTIES(${libname} PROPERTIES SUFFIX ".xop")

  TARGET_LINK_LIBRARIES(${libname} PRIVATE version.lib ${EXTRA_LIBS}
                        ${CMAKE_SOURCE_DIR}/../XOPSupport/IGOR${bitness}.lib
                        ${CMAKE_SOURCE_DIR}/../XOPSupport/VC/XOPSupport${bitness}.lib
                        optimized ${installFolderLibZMQ}/lib/libzmq-v142-mt-4_3_4.lib
                        debug ${installFolderLibZMQ}/lib/libzmq-v142-mt-gd-4_3_4.lib)

  IF(MSVC)

    # C4706: assignment within conditional expression
    # C4127: conditional expression is constant
    TARGET_COMPILE_OPTIONS(${libname} PRIVATE $<$<CONFIG:RELEASE>:/Zi> /MP /W4 /wd4706 /wd4127 $<$<BOOL:${WARNINGS_AS_ERRORS}>:/WX>)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

    FUNCTION(SET_ALL_TARGETS_LINKTYPE _dir)
      GET_PROPERTY(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
      FOREACH(_subdir IN LISTS _subdirs)
        SET_ALL_TARGETS_LINKTYPE("${_subdir}")
      ENDFOREACH()
      GET_PROPERTY(_sub_targets DIRECTORY "${_dir}" PROPERTY BUILDSYSTEM_TARGETS)

      FOREACH(_subtarget IN LISTS _sub_targets)
        GET_PROPERTY(_type TARGET ${_subtarget} PROPERTY TYPE)
        IF(NOT (${_type} STREQUAL "INTERFACE_LIBRARY"))
          SET_PROPERTY(TARGET ${_subtarget} PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${MSVC_RUNTIME_DYNAMIC}>:DLL>)
        ENDIF()
      ENDFOREACH()
    ENDFUNCTION()

    SET_ALL_TARGETS_LINKTYPE(".")
  ENDIF()
ENDIF()

SET(JSON_BuildTests OFF CACHE INTERNAL "")
SET(JSON_Install OFF CACHE INTERNAL "")
SET(JSON_ImplicitConversions OFF CACHE INTERNAL "")
ADD_SUBDIRECTORY(json)

TARGET_LINK_LIBRARIES(${libname} PRIVATE nlohmann_json::nlohmann_json)

ADD_SUBDIRECTORY(fmt EXCLUDE_FROM_ALL)

TARGET_LINK_LIBRARIES(${libname} PRIVATE fmt-header-only)

FIND_PROGRAM(CLANG_FORMAT NAMES clang-format)

IF(CLANG_FORMAT)
  # run clang-format before building
  ADD_CUSTOM_COMMAND(TARGET ${libname}
                     PRE_BUILD
                     COMMAND ${CLANG_FORMAT} -i ${SOURCES} ${HEADERS}
                     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                     COMMENT "Running clang-format" VERBATIM)
ENDIF()

FIND_PROGRAM(CLANG_TIDY PATHS ${LLVM_BREW} NAMES clang-tidy)

IF(CLANG_TIDY)
  ADD_CUSTOM_TARGET(clang-tidy
                    COMMAND ${CLANG_TIDY}
                    -fix
                    -p=${CMAKE_BINARY_DIR}
                    -export-fixes=${CMAKE_BINARY_DIR}/clang-tidy-fixes.yaml
                    ${SOURCES}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "Running clang-tidy" VERBATIM)
ENDIF()

FIND_PACKAGE(Git QUIET REQUIRED)

EXECUTE_PROCESS(
    COMMAND "${GIT_EXECUTABLE}" describe --tags --always HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_REVISION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

CONFIGURE_FILE(git_version.cpp.in git_version.cpp @ONLY)
CONFIGURE_FILE(cmake_config.h.in cmake_config.h @ONLY)

IF(MSVC)
  IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    SET(CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY TRUE)
    SET(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
  ENDIF()

  SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${installFolder})
  INCLUDE(InstallRequiredSystemLibraries)

  INSTALL(TARGETS ${libname}
          RUNTIME
          DESTINATION ${installFolder})

  INSTALL(FILES ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${libname}.pdb
          DESTINATION ${installFolder})

  INSTALL(FILES ${installFolderLibZMQ}/bin/libzmq-v142-mt-gd-4_3_4.dll
          DESTINATION ${installFolder}
          CONFIGURATIONS Debug)

  INSTALL(FILES ${installFolderLibZMQ}/bin/libzmq-v142-mt-4_3_4.dll
          DESTINATION ${installFolder}
          CONFIGURATIONS Release)

ELSEIF(APPLE)
  INSTALL(FILES ${CMAKE_BINARY_DIR}/${libname}.zip
          DESTINATION ${installFolder})
ENDIF()

IF(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
  IF(MSVC)
    create_default_target_launcher(${libname} COMMAND ${igorPath})
  ELSE()
    create_target_launcher(${libname} COMMAND ${igorPath})
  ENDIF()
ENDIF()
