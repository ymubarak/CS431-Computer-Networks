#ifndef FILEHANDLER_LIB_H
#define FILEHANDLER_LIB_H
#include<stdlib.h>
#include <stdbool.h>

typedef struct FileInfo {
    char *type;
    long size;
    char *content;
} FileInfo;

// helper functions
bool file_exists(const char *file_name);
const char *get_filename_ext(const char *filename);

struct FileInfo*  file_read(char *file_name);
void file_write(const char *file_name,const  char *data);
void free_file_data(struct FileInfo* finfo);

#endif //HTTPMESSAGE_LIB_H
