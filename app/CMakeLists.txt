include(tools)

assertVariableSet(dhtExe dhtName)

find_library(pigpio pigpio)

add_executable(${dhtExe} "")

target_sources (${dhtExe} PRIVATE main.cpp)
target_link_libraries(${dhtExe} ${dhtName} pigpio)
