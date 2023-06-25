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

function(FetchContent_MakeAvailable_With_Check targetName)
    FetchContent_MakeAvailable(${targetName})
    if(NOT ${targetName}_POPULATED)
        message(FATAL_ERROR "Could not make ${targetName} available")
    endif()
    set(${targetName}_POPULATED ${${targetName}_POPULATED} PARENT_SCOPE)
    set(${targetName}_SOURCE_DIR ${${targetName}_SOURCE_DIR} PARENT_SCOPE)
    set(${targetName}_BINARY_DIR ${${targetName}_BINARY_DIR} PARENT_SCOPE)
endfunction()

function(assertVariableSet)
    foreach(argument ${ARGN})
        if(NOT ${argument})
            message(FATAL_ERROR "Variable '${argument}' was not set")
        endif()
    endforeach()
endfunction()