#include "daemon.h"
#include "telemetry_sv.h"
#include <stdio.h>
#include <stdlib.h>

int main() 
{
    daemon_config_t config = {
        .log_level = LOG_LEVEL_DEBUG,      // Set log level to DEBUG for detailed logs
        .working_directory = "/",       // Set the working directory (can be any valid path)
        .umask_value = 027,                // Set a restrictive umask
        //.config_file = "/tmp/daemon.conf", // A placeholder path for the config file
        .run_callback = NULL, // Your application logic callback
        .user_data = NULL                  // No additional user data for this simple test
    };

    // Initialize the daemon
    if (initialize_daemon(&config) != 0) {
        log_error("Failed to initialize daemon");
        return 1;
    }

    // Run the daemon
    log_info("Daemon is running...");
    return daemon_run(&config);
    //runServer(DEFAULT_PORT);
}
