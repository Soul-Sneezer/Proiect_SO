#include "channel.h"
#include "dir.h"
#include <stdio.h>
#include <stdlib.h>

void callback1(tid_t token, const char *message) {
  printf("callback1(): [MESSAGE from %d] %s\n", token, message);
}

void callback2(tid_t token, const char *message) {
  printf("callback2(): [MESSAGE from %d] %s\n", token, message);
}

int main() {
  if (create_dir("channel_a") < 0) {
    fprintf(stderr, "Failed to open channel_a. Exiting\n");
    return -1;
  }
  if (create_dir("channel_b") < 0) {
    fprintf(stderr, "Failed to open channel_b. Exiting\n");
    return -1;
  }
  if (create_dir("channel_a/child") < 0) {
    fprintf(stderr, "Failed to open channel_a/child. Exiting\n");
    return -1;
  }

  tid_t tid_child_s = channel_open(CHANNEL_SUBSCRIBER, "channel_a/child");
  printf("Created channel 1\n");
  tid_t tid_p_a = channel_open(CHANNEL_PUBLISHER, "channel_a");
  printf("Created channel 2\n");
  tid_t tid_s_a = channel_open(CHANNEL_SUBSCRIBER, "channel_a");
  printf("Created channel 3\n");
  tid_t tid_b_b = channel_open(CHANNEL_BOTH, "channel_b");
  printf("Created channel 4\n");
  tid_t tid_s_b = channel_open(CHANNEL_SUBSCRIBER, "channel_b");
  printf("Created channel 5\n");
  channel_callback(tid_s_a, callback1);
  printf("Registered callback 1\n");
  channel_callback(tid_child_s, callback2);
  printf("Registered callback 2\n");
  channel_post(tid_p_a, "Acesta este un mesaj!");
  printf("Posted message 1\n");
  channel_post(tid_b_b, "Acesta este un mesaj in channel B!");
  printf("Posted message 2\n");
  unsigned long long mid;
  const char *message;
  message = channel_read(tid_b_b, &mid);
  printf("Mesaj %llu: %s\n", mid, message);
  free((void *)message);
  channel_post(tid_b_b, "Acesta este alt mesaj in channel B!");
  message = channel_read(tid_s_b, &mid);
  printf("Mesaj %llu: %s\n", mid, message);
  free((void *)message);

  char c;
  scanf("Press any key to continue...%c", &c);

  return 0;
}
