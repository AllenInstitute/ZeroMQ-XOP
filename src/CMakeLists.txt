CMAKE_MINIMUM_REQUIRED(VERSION 3.15 FATAL_ERROR)
# Allows setting how we link to the MSVC runtime (i.e., MSCV_RUNTIME_DYNAMIC)
cmake_policy(SET CMP0091 NEW)

# This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

if(APPLE)
  # IP8 supports MacOSX 10.10 or higher
  # https://www.wavemetrics.com/products/igorpro
  # Note:
  # Igor6 supports OSX 10.4 or later. Igor7 supports OSX 10.9 or later.
  # Igor8 supports OSX 10.10 - 10.14 (10.15 nos supported).
  # Igor9 supports 10.13 & 10.14 (and 'partially' 10.15).
  # TODO: Consider adding min/max constraints in the future?
  SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum MacOSX version")
endif()

# Limit multigenerator configs to Debug and Release for now (only ones used).
SET(CMAKE_CONFIGURATION_TYPES "Debug;Release")

PROJECT(ZeroMQ)

OPTION(COVERAGE "Enable coverage instrumentation" OFF)
OPTION(SANITIZER "Enable sanitizer instrumentation" OFF)
OPTION(MSVC_RUNTIME_DYNAMIC "Link dynamically against the MSVC runtime library" OFF)
OPTION(WARNINGS_AS_ERRORS "Error out on compiler warnings" OFF)

# Define minimum version based on XOP Toolkit. If compiling for Igor 6/7,
# set to 637 when calling cmake: cmake -DXOP_MINIMUM_IGORVERSION=637...
SET(XOP_IGOR_6_MINIMUM "637")
SET(XOP_IGOR_8_MINIMUM "800")
SET(XOP_MINIMUM_IGORVERSION ${XOP_IGOR_8_MINIMUM} CACHE STRING "Select minimum Igor Version")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/CreateLaunchers/cmake")
include(CreateLaunchers)

# Set bitness
IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
  SET(bitness "")
  SET(bitnessWithDash "")
  SET(bitnessLibFolder "x86")
ELSEIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(bitness "64")
  SET(bitnessWithDash "-64")
  SET(bitnessLibFolder "x64")
ENDIF()

# Set where we expect Igor to be, used to create our launcher.
IF(XOP_MINIMUM_IGORVERSION GREATER_EQUAL ${XOP_IGOR_8_MINIMUM})
  IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    IF(MSVC)
      SET(igorPath "C:/Program Files/WaveMetrics/Igor Pro 9 Folder/IgorBinaries_Win32/Igor.exe")
    ELSEIF(APPLE)
      MESSAGE(FATAL_ERROR "No 32bit support on MacOSX.")
    ENDIF()
  ELSEIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    IF(MSVC)
      SET(igorPath "C:/Program Files/WaveMetrics/Igor Pro 9 Folder/IgorBinaries_x64/Igor64.exe")
    ELSEIF(APPLE)
      SET(igorPath "/Applications/Igor\ Pro\ 9\ Folder/Igor64.app/Contents/MacOS/Igor64")
    ENDIF()
  ENDIF()
ELSEIF(XOP_MINIMUM_IGORVERSION GREATER_EQUAL ${XOP_IGOR_6_MINIMUM})
  IF(MSVC)
    SET(igorPath "C:/Program Files (x86)/WaveMetrics/Igor Pro Folder/Igor.exe")
  ELSEIF(APPLE)
    MESSAGE(WARNING "Igor 6/7 support is untested with Mac! Try at your own risk!")
    SET(igorPath "/Applications/Igor\ Pro\ Folder/Igor.app/Contents/MacOS/Igor")
  ENDIF()
ELSE()
  MESSAGE(SEND_ERROR "We only support Igor 6/7 >= 6.37. Not continuing.")
ENDIF()

# Set igorPath to be a cached variable, tso the user can override it.
SET(igorPath ${igorPath} CACHE FILEPATH "Path to Igor executable of interest. \
Used by launcher scripts for debugging purposes.")

SET(libname "${PROJECT_NAME}${bitnessWithDash}")

IF(APPLE)
  # use RPATH
  cmake_policy(SET CMP0042 NEW)

  SET(RESOURCES "${PROJECT_NAME}.r")
  # Indicate RESOURCES is generated, to avoid file missing checks.
  SET_SOURCE_FILES_PROPERTIES(${RESOURCES} PROPERTIES GENERATED TRUE)
  SET(RESOURCE_CONFIG "${PROJECT_NAME}.r")

  SET(installFolder "${CMAKE_SOURCE_DIR}/../output/mac/xop/$<CONFIG>")
  SET(installFolderLibZMQ "${CMAKE_SOURCE_DIR}/../output/mac/libzmq/$<CONFIG>")
ELSEIF(WIN32)
  SET(RESOURCES "${PROJECT_NAME}.rc")
  SET(RESOURCE_CONFIG "${PROJECT_NAME}WinCustom.rc")

  SET(installFolder "${CMAKE_SOURCE_DIR}/../output/win/${bitnessLibFolder}/xop/$<CONFIG>")
  SET(installFolderLibZMQ "${CMAKE_SOURCE_DIR}/../output/win/${bitnessLibFolder}/libzmq/$<CONFIG>")
ENDIF()

# Create necessary resource files from intermediaries
CONFIGURE_FILE("${RESOURCE_CONFIG}.in" ${RESOURCE_CONFIG} @ONLY)

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
SET(XOP_SUPPORT_PATH ${CMAKE_SOURCE_DIR}/../XOPSupport/ CACHE PATH
"Path to XOP Toolkit's XOPSupport directory. By default, assumed to be linked \
 at $zmq-xop-dir/XOPSupport.")

SET(XOPTOOLKIT_SOURCES
 ${XOP_SUPPORT_PATH}/IgorErrors.h
 ${XOP_SUPPORT_PATH}/IgorXOP.h
 ${XOP_SUPPORT_PATH}/NamedFIFO.h
 ${XOP_SUPPORT_PATH}/WMTypes.h
 ${XOP_SUPPORT_PATH}/XOP.h
 ${XOP_SUPPORT_PATH}/XOPContainers.c
 ${XOP_SUPPORT_PATH}/XOPContainers.h
 ${XOP_SUPPORT_PATH}/XOPDataFolderAccess.c
 ${XOP_SUPPORT_PATH}/XOPDialogsMac.c
 ${XOP_SUPPORT_PATH}/XOPDialogsWin.c
 ${XOP_SUPPORT_PATH}/XOPFileNavigationMac.c
 ${XOP_SUPPORT_PATH}/XOPFileNavigationWin.c
 ${XOP_SUPPORT_PATH}/XOPFiles.c
 ${XOP_SUPPORT_PATH}/XOPFilesMac.c
 ${XOP_SUPPORT_PATH}/XOPFilesWin.c
 ${XOP_SUPPORT_PATH}/XOPMemory.c
 ${XOP_SUPPORT_PATH}/XOPMenus.c
 ${XOP_SUPPORT_PATH}/XOPNumericConversion.c
 ${XOP_SUPPORT_PATH}/XOPResources.h
 ${XOP_SUPPORT_PATH}/XOPStandardHeaders.h
 ${XOP_SUPPORT_PATH}/XOPSupport.c
 ${XOP_SUPPORT_PATH}/XOPSupport.h
 ${XOP_SUPPORT_PATH}/XOPSupportMac.c
 ${XOP_SUPPORT_PATH}/XOPSupportMac.h
 ${XOP_SUPPORT_PATH}/XOPSupportWin.c
 ${XOP_SUPPORT_PATH}/XOPSupportWin.h
 ${XOP_SUPPORT_PATH}/XOPTextUtilityWindows.c
 ${XOP_SUPPORT_PATH}/XOPWaveAccess.c
 ${XOP_SUPPORT_PATH}/XOPWindows.c
 ${XOP_SUPPORT_PATH}/XOPWindows.h
 ${XOP_SUPPORT_PATH}/XOPWinMacSupport.h
)

FOREACH(it IN LISTS ${XOPTOOLKIT_SOURCES})
  SET_SOURCE_FILES_PROPERTIES(${it} PROPERTIES HEADER_FILE_ONLY TRUE)
ENDFOREACH()

ADD_CUSTOM_TARGET("XOP_Toolkit" SOURCES ${XOPTOOLKIT_SOURCES}
                                COMMAND "echo"
                                COMMENT "Fake target for convenience. This does nothing.")

INCLUDE_DIRECTORIES(${XOP_SUPPORT_PATH})
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
    -i ${XOP_SUPPORT_PATH}
    -i ${CMAKE_BINARY_DIR}
    -i ${CMAKE_SOURCE_DIR}
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
                         ${XOP_SUPPORT_PATH}/Xcode/libXOPSupport${bitness}.a
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
                           -fno-sanitize=unsigned-shift-base
                           -fsanitize=nullability -fno-omit-frame-pointer -fno-sanitize-recover=all
                           -fsanitize-recover=unsigned-integer-overflow>)
    TARGET_LINK_OPTIONS(${libname} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=address
                        -fsanitize=undefined -fsanitize=integer -fsanitize=nullability
                        -fno-sanitize=unsigned-shift-base
                        -fno-omit-frame-pointer -fno-sanitize-recover=all
                        -fsanitize-recover=unsigned-integer-overflow>)
  ENDIF()

  SET(LLVM_BREW "/opt/homebrew/opt/llvm/bin")

  IF(${COVERAGE})
    MESSAGE(STATUS "Building with coverage instrumentation")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-instr-generate")

    SET(coveragePath "${CMAKE_BINARY_DIR}/coverage")

    ADD_CUSTOM_TARGET(coverage-merge
                      COMMAND xcrun llvm-profdata merge -output=code.profdata default.profraw
                      WORKING_DIRECTORY ${coveragePath}
                      COMMENT "Generating coverage profraw files" VERBATIM)

    ADD_CUSTOM_TARGET(coverage-show
                      COMMAND xcrun llvm-cov show
                      ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${libname}.xop/Contents/MacOS/${libname}
                      --instr-profile=${coveragePath}/code.profdata
                      --show-instantiation-summary
                      ${COVERAGE_SOURCES}
                      --use-color --format html
                      --output-dir=${coveragePath}
                      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                      COMMENT "Generating coverage HTML" VERBATIM)

    ADD_CUSTOM_TARGET(coverage-report
                      COMMAND xcrun llvm-cov report
                      ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${libname}.xop/Contents/MacOS/${libname}
                      --instr-profile=${coveragePath}/code.profdata
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
                        ${XOP_SUPPORT_PATH}/IGOR${bitness}.lib
                        ${XOP_SUPPORT_PATH}/VC/XOPSupport${bitness}.lib
                        optimized ${installFolderLibZMQ}/lib/libzmq-v142-mt-4_3_4.lib
                        debug ${installFolderLibZMQ}/lib/libzmq-v142-mt-gd-4_3_4.lib)

  IF(MSVC)

    # C4706: assignment within conditional expression
    # C4127: conditional expression is constant
    TARGET_COMPILE_OPTIONS(${libname} PRIVATE $<$<CONFIG:RELEASE>:/Zi> /MP /W4 /wd4706 /wd4127 $<$<BOOL:${WARNINGS_AS_ERRORS}>:/WX>)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

    # This method sets the MSVC runtime linking for all non-interface library
    # dependencies (either runtime or dynamic, based on MSVC_RUNTIME_DYNAMIC).
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

FIND_PROGRAM(RUN_CLANG_TIDY PATHS ${LLVM_BREW} NAMES run-clang-tidy.py)
FIND_PROGRAM(CLANG_TIDY PATHS ${LLVM_BREW} NAMES clang-tidy)
FIND_PROGRAM(CLANG_APPLY_REPLACEMENTS PATHS ${LLVM_BREW} NAMES clang-apply-replacements)

IF(RUN_CLANG_TIDY AND CLANG_TIDY AND CLANG_APPLY_REPLACEMENTS)
  ADD_CUSTOM_TARGET(clang-tidy
                    COMMAND ${RUN_CLANG_TIDY}
                    -clang-tidy-binary=${CLANG_TIDY}
                    -clang-apply-replacements-binary=${CLANG_APPLY_REPLACEMENTS}
                    -fix
                    -p=${CMAKE_BINARY_DIR}
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
  # Get runtime libraries (debug and not) to configure for install.
  SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)  # We will call ourselves

  # Get release libraries
  INCLUDE(InstallRequiredSystemLibraries)
  SET(WIN_RUNTIME_RELEASE ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})

  # Reset module variable between calls.
  SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS "")

  # Get debug libraries
  SET(CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY TRUE)
  SET(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
  INCLUDE(InstallRequiredSystemLibraries)
  SET(WIN_RUNTIME_DEBUG ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})

  INSTALL(PROGRAMS ${WIN_RUNTIME_DEBUG}
          DESTINATION ${installFolder}
          CONFIGURATIONS Debug)
  INSTALL(PROGRAMS ${WIN_RUNTIME_RELEASE}
          DESTINATION ${installFolder}
          CONFIGURATIONS Release)

  INSTALL(TARGETS ${libname}
          RUNTIME
          DESTINATION ${installFolder})

  INSTALL(FILES ${CMAKE_BINARY_DIR}/$<CONFIG>/${libname}.pdb
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
