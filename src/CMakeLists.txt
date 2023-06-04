include(tools)

assertVariableSet(dhtName)

if (NOT MOSQUITTO_INCLUDE_DIR)
    find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h)
endif()

if (NOT MOSQUITTO_LIB)
    find_library(MOSQUITTO_LIB mosquitto)
endif()

if (NOT PIGPIO_INCLUDE_DIR)
    find_path(PIGPIO_INCLUDE_DIR pigpio.h)
endif()

if (NOT PIGPIO_LIB)
    find_library(PIGPIO_LIB pigpio)   
endif()


find_library(pigpio pigpio)

message(STATUS "MOSQUITTO_INCLUDE_DIR=${MOSQUITTO_INCLUDE_DIR}")
message(STATUS "MOSQUITTO_LIB=${MOSQUITTO_LIB}")
message(STATUS "PIGPIO_INCLUDE_DIR=${PIGPIO_INCLUDE_DIR}")
message(STATUS "PIGPIO_LIB=${PIGPIO_LIB}")

add_library(${dhtName} "")

set(myHeaders ClimateMeasurement.h Config.h Dht.h Homie.h ISender.h Mqtt.h OS.h SensorData.h)
set(mySources ClimateMeasurement.cpp Config.cpp Dht.cpp Homie.cpp Mqtt.cpp OS.cpp SensorData.cpp)
target_sources (${dhtName} PUBLIC ${myHeaders} PRIVATE ${mySources})
target_include_directories(${dhtName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${MOSQUITTO_INCLUDE_DIR} ${PIGPIO_INCLUDE_DIR})
target_link_libraries(${dhtName} ${PIGPIO_LIB} ${MOSQUITTO_LIB})
