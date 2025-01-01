#include "dir.h"

int create_dir(char *path) {

  struct stat st = {0};

  // avem nevoie de un temp pentru a accesa lungimea path-ului
  char *temp = malloc(strlen(path) + 1);
  if (!temp) {
    perror("malloc");
    free(temp);
    return -1;
  }
  // il pastram pentru moment ca null
  temp[0] = '\0';

  // ne apucam sa parsam path-ul dupa separatorul '/'
  char *p = strtok(path, "/");

  while (p) {
    if (strlen(temp) > 0) {
      strcat(temp, "/"); // concatenam path-ul catre directoarele dorite
    }
    strcat(temp, p);

    if (stat(temp, &st) == -1) {
      if (mkdir(temp, 0777) ==
          0) { // daca s-a creat ii punem si un log.txt pt evidenta mesajelor

        char *file_path = malloc(strlen(temp) + 8);
        strcpy(file_path, temp);
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

int remove_dir(char *path) {

  char *full_path;
  struct stat st_path, st_entry;
  struct dirent *entry;
  DIR *dir;

  if (stat(path, &st_path) != 0) { // verificam statusul path-ului
    perror("path status denied");
    return -1;
  }

  if (!(dir = opendir(path))) {
    if (unlink(path) == 0) {
      return 0; // am dat remove la un file;
    }
    perror("opendir/unlink");
    return -1;
  }

  while ((entry = readdir(dir)) != NULL) {

    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
      continue;
    }

    full_path = malloc(strlen(path) + 1 + strlen(entry->d_name) + 1);
    strcpy(full_path, path); // copiem path-ul actual
    strcat(full_path, "/");
    strcat(full_path, entry->d_name);

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
    } else {
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

int open_log(const char *dir_name) {

  struct stat st_path;
  if (stat(dir_name, &st_path) == -1) { // verificam sa exista path-ul
    return -1;
  }

  if (!S_ISDIR(st_path.st_mode)) { // vedem sa fie director
    return -1;
  }

  char *log_path = malloc(strlen(dir_name) + 1 + 8); // dir_name + /log.txt + 1
  if (!log_path) {
    perror("malloc");
    return -1;
  }

  strcpy(log_path, dir_name);
  strcat(log_path, "/log.txt");

  int fd = open(log_path, O_RDWR);
  free(log_path);

  if (fd == -1) {
    perror("open");
    return -1;
  }
  // daca totul a mers bine il trimitem
  return fd;
}
