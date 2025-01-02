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
#include <stddef.h>
#include <stdbool.h>
#include <fcntl.h>

typedef struct node_list
{
    char* path;
    struct node_list* next;
}node_list;

node_list* create_node(char *path);
void add_node(node_list** head, char *path);
void delete_list(node_list* head);

int create_dir(char* path);
int remove_dir(char *path);
int open_log(const char *dir_name); 
int get_dir(char* dir_path, node_list** dir_t);

#endif