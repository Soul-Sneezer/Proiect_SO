#include <stdio.h>
#include <stdlib.h>
#include "dir.h"

int main(int argc, char *argv[]) {
    
   if(argc != 3){
    printf("The command is incomplete: %s <add|rm> path\n", argv[0]);
    return 0;
   }

   if(!strcmp(argv[1],"add")){
    if(create_dir(argv[2])){
        printf("%s was created\n", argv[2]);
    }else{
        fprintf(stderr, "Failed to create directories for path: %s\n", argv[2]);
            return EXIT_FAILURE;
    }
   }else if(!strcmp(argv[1],"rm")){
    if(remove_dir(argv[2])){
        printf("%s was deleted\n", argv[2]);
    }else{
        fprintf(stderr, "Failed to remove directory: %s\n", argv[2]);
        return EXIT_FAILURE;
    }
   }else{
    printf("The command is incomplete: %s <add|rm> path\n", argv[0]);
   }


   return 0;
}