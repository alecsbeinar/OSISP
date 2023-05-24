#define DIRWALK__ARGS_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>
#include <errno.h>
using namespace std;


typedef struct {
  bool is_link;
  bool is_directory;
  bool is_file;
  bool is_sort;
} flag_t;

typedef struct{
  char type;
  char path[512];
} file_t;

typedef struct{
  size_t amount;
  file_t* data;
} files_t;



char* get_path(int argc, char* argv[]);
flag_t get_flags(int argc, char* argv[]);
bool compare_file_flags(const file_t* file, flag_t flags);
void get_files(files_t* files, char* path_directory, flag_t flags);
void Print(files_t files, flag_t flags);


int main(int argc, char** argv)
{
    char* path_directory = get_path(argc, argv);
    flag_t flags = get_flags(argc, argv);
    
    files_t files = {0, (file_t*) malloc(30000000 * sizeof(file_t))};

    get_files(&files, path_directory, flags);
    Print(files, flags);

    return 0;
}



char* get_path(int argc, char* argv[])
{
    char* path_directory = (char*)calloc(256, sizeof(char));
    (argc == 1 || argv[1][0] == '-') ? strcpy(path_directory, ".") : strcpy(path_directory, argv[1]);
    if(path_directory[strlen(path_directory) - 1] == '/') path_directory[strlen(path_directory) - 1] = '\0';
    return path_directory;
}

flag_t get_flags(int argc, char* argv[]) {
  flag_t flags = {false, false, false, false};
  int flag;
  bool exist_flags = false;
  while ((flag = getopt(argc, argv, "ldfs")) != -1) 
  {
    if (flag == 'l' || flag == 'd' || flag == 'f') exist_flags = true;
    switch (flag) 
    {
      case 'l':
        flags.is_link = true;
        break;

      case 'd':
        flags.is_directory = true;
        break;

      case 'f':
        flags.is_file = true;
        break;

      case 's':
        flags.is_sort = true;
        break;

      default:
        fprintf(stderr, "%c is invalid flag\n", flag);
        exit(EXIT_FAILURE);
    }
  }

  if (!exist_flags) 
  {
    flags.is_sort = flags.is_link = flags.is_directory = flags.is_file = true;
  }

  return flags;
}

bool compare_file_flags(const file_t* file, flag_t flags){ //return true if we need this file
  switch (file->type)
  {
    case DT_LNK:
      return flags.is_link;
    case DT_DIR:
      return flags.is_directory;
    case DT_REG:
      return flags.is_file;
    default:
      return true;
  }
}

void get_files(files_t* files, char* path_directory, flag_t flags)
{
  strcat(path_directory, "/");
  DIR* dir = opendir(path_directory);
  
    if (!dir) {
    if (errno == EACCES) {
      perror(path_directory);
      return;
    } else {
      perror(path_directory);
      exit(errno);
    }
  }

  struct dirent* dir_file;
  while((dir_file = readdir(dir))){
    if(!strcmp(dir_file->d_name, ".") || !strcmp(dir_file->d_name, ".."))
      continue;
  
    file_t file = {(char)dir_file->d_type, {'\0'}}; //copy type
    strcpy(file.path, path_directory); //copy ~
    strcat(file.path, dir_file->d_name); //concat file name


    if(compare_file_flags(&file, flags)) files->data[files->amount++] = file;
    
    //recursion entry
    if(file.type == DT_DIR) get_files(files, file.path, flags);
  
  }

  closedir(dir);

}

void Print(files_t files, flag_t flags){
  if(flags.is_sort) qsort(files.data, files.amount, sizeof(file_t), 
                          [](const void* f1, const void* f2)
                          {return strcmp(((const file_t*)f1)->path, ((const file_t*)f2)->path);});

  for(size_t i = 0; i < files.amount; ++i){
    switch (files.data[i].type)
    {
    case DT_LNK: // green
        printf("\033[1;36m%s\033[0m\n", files.data[i].path);
        break;

      case DT_DIR: // blue
        printf("\033[1;34m%s\033[0m\n", files.data[i].path);
        break;

      case DT_REG: // uncolored
        puts(files.data[i].path);
        break;

      default: // purple
        printf("\033[1;35m%s\033[0m\n", files.data[i].path);
        break;
    }
  }
}