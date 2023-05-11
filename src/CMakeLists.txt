include(tools)

assertVariableSet(dhtName)

find_library(pigpio pigpio)
message(STATUS "In Dhtx ${dhtName} src: CMAKE_PROJECT_NAME=${CMAKE_PROJECT_NAME}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}")
message(STATUS "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")

add_library(${dhtName} "")

set(myHeaders Dht.h MaxPriority.h SensorData.h)
set(mySources Dht.cpp MaxPriority.cpp SensorData.cpp)
target_sources (${dhtName} PUBLIC ${myHeaders} PRIVATE ${mySources})
target_include_directories(${dhtName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${dhtName} pigpio)
