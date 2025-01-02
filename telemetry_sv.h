#ifndef HEADER_TELEMETRY_SV // for testing the client and future integration into the daemon
#define HEADER_TELEMETRY_SV

#include "tlpi_hdr.h"
#include <stdint.h>

#define BACKLOG 100
#define DEFAULT_PORT "12000"

#define READ_OPERATION 0 // the client wants to read a message from channel
#define WRITE_OPERATION 1 // the client wants to write a message to channel

typedef struct {
    int sfd;
    //cl_list* client_list;
} tlm_sv_t;

// the server is iterative, might make it concurrent
int32_t createServer(const char* port);
int32_t closeServer(tlm_sv_t sv);
size_t getMessage(const char* path, char* message); // placeholder for function that retrieves messaje from a channel

#endif
