// Copyright 2023 Rik Essenius
// 
//   Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//   except in compliance with the License. You may obtain a copy of the License at
// 
//       http://www.apache.org/licenses/LICENSE-2.0
// 
//   Unless required by applicable law or agreed to in writing, software distributed under the License
//   is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and limitations under the License.

#ifdef _WIN32
#include <winsock.h>
#define _SC_HOST_NAME_MAX 128
#else
#include <unistd.h>
#endif

#include <cstring>
#include <algorithm>

#include "OS.h"

std::string OS::getHostName() const {
    char hostName[_SC_HOST_NAME_MAX]; // NOLINT (cert-err58-cpp) -- need a C style string
    if (gethostname(hostName, _SC_HOST_NAME_MAX) != 0) { 
        return ""; 
    }
    std::transform(hostName, hostName + strlen(hostName), hostName, ::tolower);
    return std::string(hostName);
}