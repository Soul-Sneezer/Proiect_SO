#ifndef HEADER_TELEMETRY_SV // for testing the client and future integration into the daemon
#define HEADER_TELEMETRY_SV

#include "tlpi_hdr.h"

#define BACKLOG 100
#define BUFFER_SIZE 256
#define DEFAULT_PORT "12000"

// the server is iterative, might make it concurrent
int32_t createServer(const char* port);

#endif
