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