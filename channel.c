#include "channel.h"
#include "dir.h"
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define REAL_MAX_MESSAGE_SZ MAX_MESSAGE_SZ + 256

// Function to parse log string and extract user_id and timestamp as time_t
static int parse_log_string(const char *log_string, tokid_t *token,
                            time_t *timestamp) {
  struct tm tm_time = {0};
  int result =
      sscanf(log_string, "[token=%d at %04d-%02d-%02d %02d:%02d:%02d GMT]",
             token, &tm_time.tm_year, &tm_time.tm_mon, &tm_time.tm_mday,
             &tm_time.tm_hour, &tm_time.tm_min, &tm_time.tm_sec);

  if (result != 7) {
    fprintf(stderr, "parse_log_string: Unable to parse log string\n");
    return UNKNOWN_ERROR;
  }

  // Adjust fields in struct tm
  tm_time.tm_year -= 1900; // tm_year is years since 1900
  tm_time.tm_mon -= 1;     // tm_mon is 0-based (0 = January)
  tm_time.tm_isdst = -1;   // Disable daylight saving time adjustment

  // Convert to time_t
  *timestamp = timegm(&tm_time);
  if (*timestamp == (time_t)-1) {
    fprintf(stderr, "parse_log_string: Failed to convert to time_t\n");
    return UNKNOWN_ERROR;
  }

  return SUCCESS; // Success
}

// Function to compute the message ID
static unsigned long long compute_mid(uid_t user_id, time_t timestamp) {
  long long p = 1;
  if (user_id == 0)
    p = 10;
  while (user_id) {
    p *= 10;
    user_id /= 10;
  }
  return 1LL * timestamp * p + user_id;
}

// Function to get the current GMT time
struct tm get_gmt_time() {
  time_t current_time;
  struct tm gmt_time;

  // Get the current time in seconds since the epoch
  current_time = time(NULL);
  if (current_time == (time_t)-1) {
    perror("time");
  }

  // Convert seconds to GMT
  if (gmtime_r(&current_time, &gmt_time) == NULL) {
    perror("gmtime_r");
  }

  return gmt_time;
}

typedef struct user_t {
  uid_t user_id;
  int type;
  void (*message_callback)(tokid_t token, const char *message);
  char channel_name[MAX_CHANNEL_NAME];
} user_t;

typedef struct array_t {
  user_t *buf;
  size_t size;
} array_t;

typedef struct stack_t {
  tokid_t *buf;
  size_t size;
} stack_t;

static inline void push(stack_t *stack, tokid_t value) {
  stack->buf = realloc(stack->buf, (stack->size + 1) * sizeof(*stack->buf));
  stack->buf[stack->size++] = value;
}

static inline tokid_t top(stack_t *stack) {
  return stack->buf[stack->size - 1];
}

static inline void pop(stack_t *stack) { stack->size--; }

static inline void push_back(array_t *arr, user_t *user) {
  arr->buf = realloc(arr->buf, (arr->size + 1) * sizeof(*arr->buf));
  arr->buf[arr->size++] = *user;
}

static uid_t current_uid = 0;
static user_t create_user(int type, const char *channel_name) {
  if (strlen(channel_name) >= MAX_CHANNEL_NAME) {
    fprintf(stderr, "Warning: channel name is too long. Only 255 chararcters "
                    "will be copied\n");
  }

  user_t user;
  user.user_id = current_uid++;
  user.type = type;
  user.message_callback = NULL;
  strncpy(user.channel_name, channel_name, MAX_CHANNEL_NAME);

  return user;
}

static array_t users;
static stack_t empty;

typedef struct notify_args {
  tokid_t token;
  tokid_t notifiee;
  const char *message;
} notify_args;

void *thread_notify(void *p) {
  notify_args args = *(notify_args *)p;
  if (users.buf[args.notifiee].message_callback != NULL) {
    users.buf[args.notifiee].message_callback(args.token, args.message);
  }
  return NULL;
}

// Open channel connection
tokid_t channel_open(int type, const char *channel_name) {
  if (type < CHANNEL_PUBLISHER || type > CHANNEL_BOTH) {
    fprintf(stderr, "channel_open: Unknown channel type\n");
    return UNKNOWN_CHANNEL_TYPE;
  }

  create_dir((char *)channel_name);
  printf("%s\n", channel_name);

  user_t user = create_user(type, channel_name);

  tokid_t token = users.size;
  if (empty.size > 0) {
    token = top(&empty);
    pop(&empty);

    users.buf[token].type = type;
    users.buf[token].user_id = current_uid++;
    strncpy(users.buf[token].channel_name, channel_name, MAX_CHANNEL_NAME);
  } else {
    push_back(&users, &user);
  }

  return token;
}

// Post message by channel connection with token
int channel_post(tokid_t token, const char *message) {

  // Check if message size is within parameters
  if (strlen(message) >= MAX_MESSAGE_SZ) {
    fprintf(stderr,
            "channel_post: Message too large. Maximum message size is: %d\n",
            MAX_MESSAGE_SZ);
  }

  // Get user from token
  user_t *user = &users.buf[token];
  if (user->type == CHANNEL_SUBSCRIBER) {
    fprintf(stderr,
            "channel_post: Subscribers cannot post messages to channels\n");
    return SUBSCRIBERS_CANNOT_POST;
  }

  // Check if channel is closed
  if (user->type == CHANNEL_CLOSED) {
    fprintf(stderr, "channel_post: Tlm channel marked as closed\n");
    return ERR_CHANNEL_CLOSED;
  }

  // Get log file descriptor
  int log_fd = open_log(user->channel_name);
  if (log_fd < 0) {
    fprintf(stderr, "channel_post: Error trying to get log file descriptor\n");
  }

  // Open log file with file descriptor
  FILE *log_file = fdopen(log_fd, "w+");
  if (!log_file) {
    perror("fdopen");
    return errno;
  }

  // Get timestamp
  time_t timestamp = time(NULL);
  if (timestamp == -1) {
    perror("time");
    return 1;
  }

  // Convert timestamp to GMT time and write message
  struct tm gmt_time = get_gmt_time();

  // Print message in parent channel
  fprintf(log_file, "[token=%u at %04d-%02d-%02d %02d:%02d:%02d GMT] %s\n",
          token, gmt_time.tm_year + 1900, gmt_time.tm_mon + 1, gmt_time.tm_mday,
          gmt_time.tm_hour, gmt_time.tm_min, gmt_time.tm_sec, message);

  char *long_message = malloc(REAL_MAX_MESSAGE_SZ * sizeof(*long_message) + 1);
  rewind(log_file);
  fscanf(log_file, "%[^\n]", long_message);

  // Print message in children channels
  node_list *head = malloc(sizeof(*head));
  head->path = NULL;
  head->next = NULL;
  get_dir(user->channel_name, &head);
  node_list *current = head;

  while (1) {
    if (strcmp(current->path, user->channel_name) == 0) {
      break;
    }

    printf("Opening %s\n", current->path);
    int channel_log_fd = open_log(current->path);
    if (channel_log_fd < 0) {
      fprintf(stderr,
              "channel_post: Error trying to get log file descriptor\n");
    }

    FILE *channel_log = fdopen(channel_log_fd, "w+");
    if (!channel_log) {
      perror("fdopen");
      return errno;
    }

    fprintf(channel_log, "[token=%u at %04d-%02d-%02d %02d:%02d:%02d GMT] %s\n",
            token, gmt_time.tm_year + 1900, gmt_time.tm_mon + 1,
            gmt_time.tm_mday, gmt_time.tm_hour, gmt_time.tm_min,
            gmt_time.tm_sec, message);

    fclose(channel_log);
    close(channel_log_fd);

    current = current->next;
  }
  delete_list(head);

  // Notify users who registered callbacks, in separate threads in case some
  // are faulty
  for (size_t i = 0; i < users.size; i++) {
    if (users.buf[i].message_callback != NULL &&
        strstr(users.buf[i].channel_name, user->channel_name)) {
      notify_args args = {token, (tokid_t)i, long_message};
      pthread_t thread;

      if (pthread_create(&thread, NULL, thread_notify, &args) != 0) {
        perror("pthread_create");
        return errno;
      }
      if (pthread_join(thread, NULL) != 0) {
        perror("pthread_join");
        return errno;
      }
    }
  }

  free(long_message);
  fclose(log_file);
  close(log_fd);
  return SUCCESS;
}

// Read message from channel connection with token
const char *channel_read(tokid_t token, unsigned long long *message_id) {
  // Get user from token
  user_t *user = &users.buf[token];
  if (user->type == CHANNEL_PUBLISHER) {
    fprintf(stderr,
            "channel_read: Publishers cannot read messages from channels\n");
    return PUBLISHERS_CANNOT_READ;
  }

  // Check if channel is closed
  if (user->type == CHANNEL_CLOSED) {
    fprintf(stderr, "channel_read: Tlm channel marked as closed\n");
    return NULL;
  }

  // Get log file descriptor
  int log_fd = open_log(user->channel_name);
  if (log_fd < 0) {
    fprintf(stderr, "channel_read: Error trying to get log file descriptor\n");
    return NULL;
  }

  // Open log file with file descriptor
  FILE *log_file = fdopen(log_fd, "r");
  if (!log_file) {
    perror("channel_open: fdopen: ");
    return NULL;
  }

  // Read message from log file
  char *message = malloc(REAL_MAX_MESSAGE_SZ * sizeof(*message));
  char newline = '\n';
  fscanf(log_file, "%[^\n]%c", message, &newline);

  // Get timestamp and uid from message
  time_t timestamp;
  tokid_t message_token;
  int ret = parse_log_string(message, &message_token, &timestamp);
  if (ret < 0) {
    fprintf(stderr, "channel_read: Failed to parse log string\n");
    return NULL;
  }

  // Compute message id
  if (message_id != NULL)
    *message_id = compute_mid(users.buf[message_token].user_id, timestamp);

  fclose(log_file);
  close(log_fd);
  return (const char *)message;
}

// Close channel connection
int channel_close(tokid_t token) {

  // Mark as closed and push to empty (available) channel stack
  users.buf[token].type = CHANNEL_CLOSED;
  users.buf[token].message_callback = NULL;
  push(&empty, token);
  return SUCCESS;
}

int channel_callback(tokid_t token,
                     void (*message_callback)(tokid_t token,
                                              const char *message)) {
  users.buf[token].message_callback = message_callback;
  return SUCCESS;
}
