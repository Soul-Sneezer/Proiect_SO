#ifndef DAEMON_H
#define DAEMON_H

#include "telemetry_sv.h"

#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum
{
  LOG_LEVEL_ERROR = 0,
  LOG_LEVEL_INFO,
  LOG_LEVEL_DEBUG
} log_level_t;

typedef struct
{
  log_level_t log_level;
  const char *working_directory;
  mode_t umask_value;
  const char *config_file;
  void (*run_callback) (void *user_data);
  void *user_data;
} daemon_config_t;

static log_level_t current_log_level;

typedef int (*config_reload_handler_t) (daemon_config_t *config);
static config_reload_handler_t user_reload_handler = NULL;

static inline void set_log_level (log_level_t level)
{
  current_log_level = level;
}

static inline void log_message (log_level_t level, const char *format, ...)
{
  if (level > current_log_level)
  {
    return;
  }

  va_list args;
  va_start (args, format);

  FILE *output = (level == LOG_LEVEL_ERROR) ? stderr : stdout;

  switch (level)
  {
  case LOG_LEVEL_ERROR:
    fprintf (output, "ERROR: ");
    break;
  case LOG_LEVEL_INFO:
    fprintf (output, "INFO: ");
    break;
  case LOG_LEVEL_DEBUG:
    fprintf (output, "DEBUG: ");
    break;
  default:
    break;
  }

  vfprintf (output, format, args);
  fprintf (output, "\n");
  fflush (output);

  va_end (args);
}

#define log_error(...) log_message (LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_info(...) log_message (LOG_LEVEL_INFO, __VA_ARGS__)
#define log_debug(...) log_message (LOG_LEVEL_DEBUG, __VA_ARGS__)

static volatile int daemon_running = 1;
static volatile int reconfiguration_requested = 0;

typedef int (*sd_notify_func_t) (int unset_environment, const char *state);
static sd_notify_func_t sd_notify_ptr = NULL;

static void initialize_sd_notify (void)
{
  if (getenv ("NOTIFY_SOCKET") == NULL)
  {
    return;
  }

  void *handle = dlopen ("libsystemd.so.0", RTLD_NOW);
  if (!handle)
  {
    log_error ("Failed to load libsystemd: %s", dlerror ());
    return;
  }

  dlerror ();

  sd_notify_ptr = (sd_notify_func_t)dlsym (handle, "sd_notify");
  char *error = dlerror ();
  if (error != NULL)
  {
    log_error ("Failed to load symbol sd_notify: %s", error);
    sd_notify_ptr = NULL;
    dlclose (handle);
    return;
  }
}

static inline int my_sd_notify (int unset_environment, const char *state)
{
  if (sd_notify_ptr)
  {
    return sd_notify_ptr (unset_environment, state);
  }
  return 0;
}

// Main loop should handle graceful termination and config file reload
static void handle_signal (int sig)
{
  switch (sig)
  {
  case SIGTERM:
    log_info ("Received SIGTERM, initiating shutdown");
    my_sd_notify (0, "STOPPING=1");
    daemon_running = 0;
    break;
  case SIGHUP:
    log_info ("Received SIGHUP, requesting configuration reload");
    my_sd_notify (0, "RELOADING=1");
    reconfiguration_requested = 1;
    break;
  }
}

static inline int initialize_daemon (const daemon_config_t *config)
{
  if (!config)
  {
    return -1;
  }

  set_log_level (config->log_level);

  struct sigaction sa = {.sa_handler = handle_signal, .sa_flags = 0};
  sigemptyset (&sa.sa_mask);

  if (sigaction (SIGTERM, &sa, NULL) < 0)
  {
    log_error ("Failed to handle SIGTERM: %s", strerror (errno));
    return -1;
  }
  if (sigaction (SIGHUP, &sa, NULL) < 0)
  {
    log_error ("Failed to handle SIGHUP: %s", strerror (errno));
    return -1;
  }

  if (config->working_directory && chdir (config->working_directory) < 0)
  {
    log_error ("Failed to change working directory: %s", strerror (errno));
    return -1;
  }

  umask (config->umask_value);
  initialize_sd_notify ();
  my_sd_notify (0, "READY=1");
  log_info ("Daemon initialization complete");

  return 0;
}

static inline int parse_config_file (const char *filename,
                                     void (*handler) (const char *key,
                                                      const char *value,
                                                      void *user_data),
                                     void *user_data)
{
  FILE *f = fopen (filename, "r");
  if (!f)
    return -1;

  char line[1024];
  while (fgets (line, sizeof (line), f))
  {
    char *key = strtok (line, "=");
    char *value = strtok (NULL, "\n");

    if (key && value)
    {
      while (isspace (*key))
        key++;
      while (isspace (*value))
        value++;

      char *end = key + strlen (key) - 1;
      while (end > key && isspace (*end))
        *end-- = '\0';

      end = value + strlen (value) - 1;
      while (end > value && isspace (*end))
        *end-- = '\0';

      handler (key, value, user_data);
    }
  }

  fclose (f);
  return 0;
}

static inline void register_config_handler (config_reload_handler_t handler)
{
  user_reload_handler = handler;
}

static inline bool is_systemd_service (void)
{
  return getenv ("NOTIFY_SOCKET") != NULL;
}

static inline bool daemon_is_running (void) { return daemon_running; }

static inline bool daemon_should_reload (void)
{
  bool should_reload = reconfiguration_requested;
  reconfiguration_requested = 0;
  return should_reload;
}

static inline void daemon_notify_status (const char *status)
{
  my_sd_notify (0, status);
}

static inline int daemon_run (daemon_config_t *config)
{
  if (!config)
    return -1;

  while (daemon_is_running ())
  {
    if (daemon_should_reload ())
    {
      daemon_notify_status ("STATUS=Reloading configuration");

      if (user_reload_handler)
      {
        if (user_reload_handler (config) != 0)
        {
          log_error ("Failed to reload configuration");
        }
        else
        {
          log_info ("Configuration reloaded successfully");
        }
      }

      daemon_notify_status ("READY=1");
    }

    if (config->run_callback)
    {
      config->run_callback (config->user_data);
    }

    runServer(DEFAULT_PORT);

    daemon_notify_status ("STATUS=Running normally");
    sleep (1);
  }

  return 0;
}

#endif // DAEMON_H
