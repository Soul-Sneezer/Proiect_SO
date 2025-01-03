#include "channel.h"
#include "dir.h"
#include <stdio.h>
#include <stdlib.h>

void callback1(tlm_t token, const char *message) {
  printf("callback1(): [MESSAGE from %d] %s\n", token, message);
}

void callback2(tlm_t token, const char *message) {
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

  tlm_t tid_child_s = tlm_open(TLM_SUBSCRIBER, "channel_a/child");
  printf("Created channel 1\n");
  tlm_t tid_p_a = tlm_open(TLM_PUBLISHER, "channel_a");
  printf("Created channel 2\n");
  tlm_t tid_s_a = tlm_open(TLM_SUBSCRIBER, "channel_a");
  printf("Created channel 3\n");
  tlm_t tid_b_b = tlm_open(TLM_BOTH, "channel_b");
  printf("Created channel 4\n");
  tlm_t tid_s_b = tlm_open(TLM_SUBSCRIBER, "channel_b");
  printf("Created channel 5\n");
  tlm_callback(tid_s_a, callback1);
  printf("Registered callback 1\n");
  tlm_callback(tid_child_s, callback2);
  printf("Registered callback 2\n");
  tlm_post(tid_p_a, "Acesta este un mesaj!");
  printf("Posted message 1\n");
  tlm_post(tid_b_b, "Acesta este un mesaj in channel B!");
  printf("Posted message 2\n");
  unsigned long long mid;
  const char *message;
  message = tlm_read(tid_b_b, &mid);
  printf("Mesaj %llu: %s\n", mid, message);
  free((void *)message);
  tlm_post(tid_b_b, "Acesta este alt mesaj in channel B!");
  message = tlm_read(tid_s_b, &mid);
  printf("Mesaj %llu: %s\n", mid, message);
  free((void *)message);

  char c;
  scanf("Press any key to continue...%c", &c);

  return 0;
}
