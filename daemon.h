#ifndef DAEMON_H
#define DAEMON_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} log_level_t;
typedef struct {
    log_level_t log_level;
    const char *working_directory;
    mode_t umask_value;
    const char *config_file;
    void (*run_callback)(void *user_data);
    void *user_data;
} daemon_config_t;

static log_level_t current_log_level;

typedef int (*config_reload_handler_t)(daemon_config_t *config);
static config_reload_handler_t user_reload_handler = NULL;

static inline void set_log_level(log_level_t level) {
    current_log_level = level;
}

static inline void log_message(log_level_t level, const char *format, ...) {
    if (level > current_log_level) {
        return;
    }

    va_list args;
    va_start(args, format);

    FILE *output = (level == LOG_LEVEL_ERROR) ? stderr : stdout;

    switch (level) {
        case LOG_LEVEL_ERROR:
            fprintf(output, "ERROR: ");
            break;
        case LOG_LEVEL_INFO:
            fprintf(output, "INFO: ");
            break;
        case LOG_LEVEL_DEBUG:
            fprintf(output, "DEBUG: ");
            break;
        default:
            break;
    }
    
    vfprintf(output, format, args);
    fprintf(output, "\n");
    fflush(output);

    va_end(args);
}

#define log_error(...) log_message(LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_info(...)  log_message(LOG_LEVEL_INFO, __VA_ARGS__)
#define log_debug(...) log_message(LOG_LEVEL_DEBUG, __VA_ARGS__)

#endif // DAEMON_H
