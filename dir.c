#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

struct stat st = {0};

int create_dir(char* path){

    //avem nevoie de un temp pentru a accesa lungimea path-ului
    char *temp = malloc(strlen(path) + 1);
    if(!temp){
        perror("malloc");
        free(temp);
        return -1;
    }
    //il pastram pentru moment ca null
    temp[0] = '\0';


    //ne apucam sa parsam path-ul dupa separatorul '/'
    char *p = strtok(path,"/");

    while(p){
        if(strlen(temp) > 0){
            strcat(temp,"/");// concatenam path-ul catre directoarele dorite
        } 
        strcat(temp, p);

        if(stat(temp,&st) == -1){
            if(mkdir(temp, 0777) == 0){//daca s-a creat ii punem si un log.txt pt evidenta mesajelor

                char *file_path = malloc(strlen(temp)+8);
                strcpy(file_path,temp);
                strcat(file_path, "/log.txt");
                FILE *log = fopen(file_path, "w");
                
                if (log == NULL) {
                perror("fopen for log.txt");
                free(temp);
                
                return -1;
                }
            fclose(log);
            }
        }

        p = strtok(NULL, "/");
    }
    free(temp);
    return 1;
}

int remove_dir(char *path){
    
    char* full_path;
    struct stat st_path, st_entry;
    struct dirent *entry;
    DIR *dir;

    if(stat(path, &st_path) != 0){ //verificam statusul path-ului
        perror("path status denied");
        return -1;
    }


    if(!(dir=opendir(path))){
        if(unlink(path) == 0){
            return 0; //am dat remove la un file;
        }
        perror("opendir/unlink");
        return -1;
    }

    while((entry = readdir(dir))!=NULL){
        
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name,"..")){
            continue;
        }

        full_path = malloc(strlen(path) + 1 + strlen(entry->d_name) + 1);
        strcpy(full_path, path); //copiem path-ul actual
        strcat(full_path,"/");
        strcat(full_path,entry->d_name);


        if (lstat(full_path, &st_entry) == -1) {
            printf("%s \n", full_path);
            perror("lstat");
            closedir(dir);
            return -1;
        }

        if (S_ISDIR(st_entry.st_mode)) {
            if (remove_dir(full_path) == -1) {
                closedir(dir);
                return -1;
            }
        }else{
            if (unlink(full_path) == -1) {
                perror("unlink");
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);

    if (rmdir(path) == -1) {
        perror("rmdir");
        return -1;
    }

    return 1;
}



int main(int argc, char *argv[]) {
   if(argc != 3){
    printf("The command is incomplete: %s <add|rm> path", argv[0]);
    return 0;
   }

   if(!strcmp(argv[1],"add")){
    if(create_dir(argv[2])){
        printf("%s was created\n", argv[2]);
    }else{
        fprintf(stderr, "Failed to create directories for path: %s\n", argv[2]);
            return EXIT_FAILURE;
    }
   }


   if(!strcmp(argv[1],"rm")){
    if(remove_dir(argv[2])){
        printf("%s was deleted\n", argv[2]);
    }else{
        fprintf(stderr, "Failed to remove directory: %s\n", argv[2]);
        return EXIT_FAILURE;
    }
   }

   return 0;
}
