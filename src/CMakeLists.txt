include(tools)

assertVariableSet(dhtName)

find_library(pigpio pigpio)
find_library(mosquittopp mosquittopp)

message(STATUS "In ${dhtName} src: CMAKE_PROJECT_NAME=${CMAKE_PROJECT_NAME}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}")
message(STATUS "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")

add_library(${dhtName} "")

set(myHeaders ClimateMeasurement.h Config.h Dht.h Homie.h ISender.h Mqtt.h SensorData.h)
set(mySources ClimateMeasurement.cpp Config.cpp Dht.cpp Homie.cpp Mqtt.cpp SensorData.cpp)
target_sources (${dhtName} PUBLIC ${myHeaders} PRIVATE ${mySources})
target_include_directories(${dhtName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${dhtName} pigpio mosquittopp)
