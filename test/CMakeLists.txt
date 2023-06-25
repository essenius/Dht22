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

include(tools)

include(FindGit)
find_package(Git)

assertVariableSet(dhtName dhtTestName Git_FOUND)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG release-1.12.1
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
    INSTALL_COMMAND ""
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable_With_Check(googletest)

add_executable(${dhtTestName} "")

#file(GLOB_RECURSE myHeaders LIST_DIRECTORIES true *.h)
#file(GLOB_RECURSE mySources LIST_DIRECTORIES true *.cpp)

set(myHeaders "")
set(mySources ConfigTest.cpp MqttTest.cpp SensorDataTest.cpp Main.cpp)

target_sources (${dhtTestName} PRIVATE ${myHeaders} PRIVATE ${mySources})

target_link_libraries(${dhtTestName} ${dhtName} gtest_main)

add_test(NAME ${dhtTestName} COMMAND ${dhtTestName})
