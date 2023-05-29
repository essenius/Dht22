include(tools)

assertVariableSet(dhtName)

if (NOT MOSQUITTOPP_INCLUDE_DIR)
    find_path(MOSQUITTOPP_INCLUDE_DIR mosquitto.h)
endif()

if (NOT MOSQUITTOPP_LIB)
    find_library(MOSQUITTOPP_LIB mosquittopp)
endif()

if (NOT PIGPIOD_INCLUDE_DIR)
    find_path(PIGPIOD_INCLUDE_DIR pigpiod_if2.h)
endif()

if (NOT PIGPIOD_LIB)
    find_library(PIGPIOD_LIB pigpiod_if2)   
endif()

message(STATUS "MOSQUITTOPP_INCLUDE_DIR=${MOSQUITTOPP_INCLUDE_DIR}")
message(STATUS "MOSQUITTOPP_LIB=${MOSQUITTOPP_LIB}")
message(STATUS "PIGPIOD_INCLUDE_DIR=${PIGPIOD_INCLUDE_DIR}")
message(STATUS "PIGPIOD_LIB=${PIGPIOD_LIB}")

add_library(${dhtName} "")

set(myHeaders ClimateMeasurement.h Config.h Dht.h Homie.h ISender.h Mqtt.h OS.h SensorData.h)
set(mySources ClimateMeasurement.cpp Config.cpp Dht.cpp Homie.cpp Mqtt.cpp OS.cpp SensorData.cpp)
target_sources (${dhtName} PUBLIC ${myHeaders} PRIVATE ${mySources})
target_include_directories(${dhtName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${MOSQUITTOPP_INCLUDE_DIR} ${PIGPIOD_INCLUDE_DIR})
target_link_libraries(${dhtName} ${PIGPIOD_LIB} ${MOSQUITTOPP_LIB})
