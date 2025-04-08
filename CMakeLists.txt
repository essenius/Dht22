# Copyright 2023 Rik Essenius
# 
#   Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of the License at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
#   Unless required by applicable law or agreed to in writing, software distributed under the License
#   is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and limitations under the License.

cmake_minimum_required(VERSION 3.14)

set(dhtName Dht)
set(dhtExe ${dhtName}Run)
set(dhtTestName ${dhtName}Test)

project(${dhtName} VERSION 0.0.14 LANGUAGES CXX)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(TOP_LEVEL ON)
else()
  set(TOP_LEVEL OFF)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(CMAKE_COMPILER_IS_GNUCXX)
	add_compile_options(-Wall -Wextra -Wpedantic -Werror -O0 -g)
endif()

message(STATUS "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")

if (WIN32)	
  # this does not work right yet. For some reason the linker cannot find the pigpio-mock library
  message(STATUS "Windows build")
  set(CMAKE_PREFIX_PATH $ENV{MOSQUITTO_DIR} $ENV{MOSQUITTO_DIR}/devel $ENV{PIGPIO_DIR}; $ENV{PIGPIO_DIR}/build)
  message(STATUS "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")
  #set(CMAKE_INCLUDE_PATH $ENV{LOCALAPPDATA}/mosquitto/devel ${CMAKE_INCLUDE_PATH})
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/lib;${PROJECT_BINARY_DIR}/lib/cmake")

if (TOP_LEVEL) 
  message(STATUS "Top level project - enabling code coverage")
  add_subdirectory(lib)
#  set(CODE_COVERAGE ON)
#  include(code-coverage)
#  add_code_coverage()
endif()

add_subdirectory(src)

add_subdirectory(app)

if(TOP_LEVEL)
  message(STATUS "Top level project - enabling tests")
  enable_testing()
  add_subdirectory(test)
#  # target_code_coverage(${dhtTestName} EXCLUDE build/_deps/googletest-src/* test/*) 
endif()

# For testing on on Windows, run: 
#  mkdir build
#  cd build
#  cmake -G "MinGW Makefiles" ..
#  mingw32-make.exe
#  ctest --output-on-failure
# Make sure Mosquitto and the pigpio-mock library are in the PATH
# to generate coverage report run 'make ccov-DhtTest' and view build/ccov/DhtTest/index.html