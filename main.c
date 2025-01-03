#include <stdio.h>
#include <stdlib.h>
#include "dir.h"

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("The command is incomplete: %s <add|rm|check> path\n", argv[0]);
        return 0;
    }

    if (!strcmp(argv[1], "add"))
    {
        if (create_dir(argv[2]))
        {
            printf("%s was created\n", argv[2]);
        }
        else
        {
            fprintf(stderr, "Failed to create directories for path: %s\n", argv[2]);
            return EXIT_FAILURE;
        }
    }
    else if (!strcmp(argv[1], "rm"))
    {
        if (remove_dir(argv[2]))
        {
            printf("%s was deleted\n", argv[2]);
        }
        else
        {
            fprintf(stderr, "Failed to remove directory: %s\n", argv[2]);
            return EXIT_FAILURE;
        }
    }
    else if (!strcmp(argv[1], "check"))
    {
        int fd = open_log(argv[2]);
        if (fd == -1)
        {
            printf("The path does not exist!\n");
        }
        else
        {
            printf("The path exists!\n");
            write(fd, "Succes", strlen("Succes"));
        }
        close(fd);
    }
    else if (!strcmp(argv[1], "get"))
    {
        node_list *dir_t = NULL;
        if (get_dir(argv[2], &dir_t) == -1)
        {
            printf("Erorr");
            return -1;
        }
        else
        {
            while (dir_t)
            {
                printf("%s\n", dir_t->path);
                dir_t = dir_t->next;
            }
        }
        delete_list(dir_t);
    }
    else
    {
        printf("The command is incomplete: %s <add|rm|check> path\n", argv[0]);
    }

    return 0;
}