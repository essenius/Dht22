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
add_compile_options(-Wall -Wextra -Wpedantic -Werror -O0 -g)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/lib;${PROJECT_BINARY_DIR}/lib/cmake")

#if (TOP_LEVEL) 
#  message(STATUS "Top level project - enabling code coverage")
#  add_subdirectory(lib)
#  set(CODE_COVERAGE ON)
#  include(code-coverage)
#  add_code_coverage()
#endif()

add_subdirectory(src)

add_subdirectory(app)

#if(TOP_LEVEL)
#  message(STATUS "Top level project - enabling tests")
#  enable_testing()
#  add_subdirectory(test)
#  # target_code_coverage(${dhtTestName} EXCLUDE build/_deps/googletest-src/* test/*) 
#endif()

# to generate coverage report run 'make ccov-DhtTest' and view build/ccov/DhtTest/index.html