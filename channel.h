#ifndef CHANNEL_H
#define CHANNEL_H

#define MAX_CHANNEL_NAME 255
#define MAX_MESSAGE_SZ 4096

#define CHANNEL_CLOSED -1
#define CHANNEL_PUBLISHER 0x1
#define CHANNEL_SUBSCRIBER 0x2
#define CHANNEL_BOTH 0x3

#define SUCCESS 0
#define UNKNOWN_CHANNEL_TYPE -1
#define SUBSCRIBERS_CANNOT_POST -2
#define MESSAGE_TOO_BIG -3
#define UNKNOWN_ERROR -4
#define PUBLISHERS_CANNOT_READ NULL
#define ERR_CHANNEL_CLOSED -5
#define FAILED_CHANNEL_OPEN -6

typedef int tid_t;

tid_t channel_open(int type, const char *channel_name);

int channel_callback(tid_t token,
                     void (*message_callback)(tid_t token,
                                              const char *message));

const char *channel_read(tid_t token, unsigned long long *message_id);

int channel_post(tid_t token, const char *message);

int channel_close(tid_t token);

#endif
