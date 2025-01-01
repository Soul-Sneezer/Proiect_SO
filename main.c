#include "channel.h"
#include "dir.h"
#include <stdio.h>
#include <stdlib.h>

void callback(tlm_t token, const char *message) {
  printf("[MESSAGE from %d] %s\n", token, message);
}

int main(int argc, char *argv[]) {
  if (create_dir("channel_a") < 0) {
    fprintf(stderr, "Failed to open channel_a. Exiting\n");
    return -1;
  }
  if (create_dir("channel_b") < 0) {
    fprintf(stderr, "Failed to open channel_b. Exiting\n");
    return -1;
  }
  tlm_t tid_p_a = tlm_open(TLM_PUBLISHER, "channel_a");
  tlm_t tid_s_a = tlm_open(TLM_SUBSCRIBER, "channel_a");
  tlm_t tid_b_b = tlm_open(TLM_BOTH, "channel_b");
  tlm_t tid_s_b = tlm_open(TLM_SUBSCRIBER, "channel_b");
  tlm_callback(tid_s_a, callback);
  tlm_post(tid_p_a, "Acesta este un mesaj!");
  tlm_post(tid_b_b, "Acesta este un mesaj in channel B!");
  unsigned long long mid;
  const char *message;
  message = tlm_read(tid_b_b, &mid);
  printf("Mesaj %llu: %s\n", mid, message);
  free((void *)message);
  tlm_post(tid_b_b, "Acesta este alt mesaj in channel B!");
  message = tlm_read(tid_s_b, &mid);
  printf("Mesaj %llu: %s\n", mid, message);
  free(message);

  char c;
  scanf("Press any key to continue...%c", &c);

  tlm_close(tid_p_a);
  printf("Closed publisher\n");
  tlm_close(tid_s_a);
  printf("Closed subscriber\n");
  tlm_t tid = tlm_open(TLM_BOTH, "channel_a");
  return 0;
}
