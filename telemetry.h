#ifndef TELEMETRY_LIB
#define TELEMETRY_LIB

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "tlpi_hdr.h"

#define TLM_PUBLISHER 0x1
#define TLM_SUBSCRIBER 0x2
#define TLM_BOTH 0x3
#define TLM_CLOSED 0x4

#define DAEMON_DEFAULT_PORT "12000"
#define MAX_PATH_LENGTH 256
#define MAX_MESSAGE_LENGTH 4096

typedef struct {
    int type;

    int sfd;
    time_t timestamp;
    uint32_t user;

    uint8_t channel_len;
    char* channel_path;
} tlm_t; 

tlm_t tlm_open(uint8_t type, const char* channel_name, const char* ip, const char* port);
int32_t tlm_callback(tlm_t token, void (*message_callback)(tlm_t token, const char* message));
const char* tlm_read(tlm_t token, uint32_t* message_id);
int32_t tlm_post(tlm_t token, const char* message);
void tlm_close(tlm_t token);
int32_t tlm_type(tlm_t token);

#endif
