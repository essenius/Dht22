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
