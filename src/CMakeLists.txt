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

assertVariableSet(dhtName)

if (NOT MOSQUITTO_INCLUDE_DIR)
    find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h)
endif()

if (NOT MOSQUITTO_LIB)
    message(STATUS "Searching. CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")
    find_library(MOSQUITTO_LIB mosquitto REQUIRED)
endif()

if (NOT PIGPIO_INCLUDE_DIR)
    find_path(PIGPIO_INCLUDE_DIR pigpio.h)
endif()

if (NOT PIGPIO_LIB)
    find_library(PIGPIO_LIB pigpio)   
endif()

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
