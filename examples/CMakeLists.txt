CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

SET(prog "zmq_xop_client")

PROJECT(${prog})
ADD_EXECUTABLE(${prog} cli-client.cpp)

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../src/libzmq/include")
SET_TARGET_PROPERTIES(${prog} PROPERTIES CXX_STANDARD 11)

# (Taken from src/CMakeLists.txt, to determine libzmq path. Yes, this could be
# copied into its own CMake module, but I'm not sure it's necessary right now...)
IF(APPLE)
  SET(installFolderLibZMQ "${CMAKE_SOURCE_DIR}/../output/mac/libzmq/${CMAKE_BUILD_TYPE}")
ELSEIF(WIN32)
  IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    SET(bitnessLibFolder "x86")
  ELSEIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(bitnessLibFolder "x64")
  ENDIF()
  SET(installFolderLibZMQ "${CMAKE_SOURCE_DIR}/../output/win/${bitnessLibFolder}/libzmq/${CMAKE_BUILD_TYPE}")
ENDIF()

TARGET_LINK_LIBRARIES(${prog}
                      optimized ${installFolderLibZMQ}/lib/libzmq-v142-mt-4_3_4.lib
                      debug ${installFolderLibZMQ}/lib/libzmq-v142-mt-gd-4_3_4.lib)
