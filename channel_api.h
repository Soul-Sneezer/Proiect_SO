#ifndef CHANNEL_API_H
#define CHANNEL_API_H

#define MAX_CHANNEL_NAME 255
#define MAX_MESSAGE_SZ 4096

#define TLM_CLOSED -1
#define TLM_PUBLISHER 0x1
#define TLM_SUBSCRIBER 0x2
#define TLM_BOTH 0x3

#define SUCCESS 0
#define UNKNOWN_CHANNEL_TYPE -1
#define SUBSCRIBERS_CANNOT_POST -2
#define MESSAGE_TOO_BIG -3
#define UNKNOWN_ERROR -4
#define PUBLISHERS_CANNOT_READ NULL
#define CHANNEL_CLOSED -5

typedef int tlm_t;

tlm_t tlm_open(int type, const char *channel_name);

int tlm_callback(tlm_t token,
                 void (*message_callback)(tlm_t token, const char *message));

const char *tlm_read(tlm_t token, unsigned long long *message_id);

int tlm_post(tlm_t token, const char *message);

int tlm_close(tlm_t token);

#endif
