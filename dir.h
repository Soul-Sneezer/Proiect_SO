#ifndef DIR_H
#define DIR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>


int create_dir(char* path);
int remove_dir(char *path);

#endif