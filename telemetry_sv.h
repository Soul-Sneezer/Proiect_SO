#ifndef HEADER_TELEMETRY_SV // for testing the client and future integration into the daemon
#define HEADER_TELEMETRY_SV

#include "tlpi_hdr.h"
#include <stdint.h>

#define BACKLOG 100
#define BUFFER_SIZE 256
#define DEFAULT_PORT "12000"

#define NEW_SIZE(size) ((size) == 0 ? 8 : ((size) * 2)

#define READ_OPERATION 0 // the client wants to read a message from channel
#define WRITE_OPERATION 1 // the client wants to write a message to channel

typedef struct {
    int* cfds;
    uint32_t count;
    uint32_t size;
} cl_list;

typedef struct {
    int sfd;
    cl_list* client_list;
} tlm_sv_t;

// the server is iterative, might make it concurrent
int32_t createServer(const char* port);

#endif
