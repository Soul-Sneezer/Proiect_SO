#ifndef HEADER_TELEMETRY_SV // for testing the client and future integration into the daemon
#define HEADER_TELEMETRY_SV

#include "tlpi_hdr.h"
#include "dynamic_list.h"
#include <stdint.h>

#define BACKLOG 100
#define DEFAULT_PORT "12000"

#define READ_OPERATION 0 // the client wants to read a message from channel
#define WRITE_OPERATION 1 // the client wants to write a message to channel
#define REGISTER_CHANNEL 2
#define CLOSE_CHANNEL 3
#define SUBSCRIBE_OPERATION 4
#define NOTIFY_OPERATION 5

typedef struct {
    int sfd;
    List* client_list; // list of people to notify
} tlm_sv_t;

// the server is iterative, might make it concurrent
int32_t runServer(const char* port);
int32_t closeServer(tlm_sv_t sv);

#endif
