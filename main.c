#include "daemon.h"
#include <stdio.h>
#include <stdlib.h>

int main() 
{
    daemon_config_t conf;
    set_log_level(LOG_LEVEL_INFO);
    daemon_run(&conf);
}
