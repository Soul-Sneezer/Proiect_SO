#ifndef TELEMETRY_LIB
#define TELEMETRY_LIB

#include <stdint.h>
#include  <stdbool.h>
#include "tlpi_hdr.h"

#define TLM_PUBLISHER 0x1
#define TLM_SUBSCRIBER 0x2
#define TLM_BOTH 0x3

#define DAEMON_DEFAULT_PORT 12001

typedef struct {
    int sfd;
    char* channel_path;
}tlm_t; // ma gandesc ca tlm_t poate fi doar id-ul canalului
                       // signed pentru error reporting

tlm_t tlm_open(uint8_t type, const char* channel_name, const char* ip, const char* port);
int32_t tlm_callback(tlm_t token, void (*message_callback)(tlm_t token, const char* message));
const char* tlm_read(tlm_t token, uint32_t* message_id);
int32_t tlm_post(tlm_t token, const char* message);
void tlm_close(tlm_t token);
int32_t tlm_type(tlm_t token);

#endif
