#ifndef HEADER_TELEMETRY_SV // for testing the client and future integration into the daemon
#define HEADER_TELEMETRY_SV

#include "tlpi_hdr.h"

// the server is iterative, might make it concurrent
int32_t createServer(const char* port);

#endif
