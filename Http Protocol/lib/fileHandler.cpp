#include<stdlib.h>
#include<stdio.h>
#include <string.h>
#include<sys/stat.h>
#include "fileHandler.h"

bool file_exists (const char *file_name){
    struct stat buffer;
    return (stat (file_name, &buffer) == 0); 
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

struct FileInfo* file_read(char *file_name){
    struct FileInfo* finfo = (struct FileInfo*) malloc(sizeof(struct FileInfo));
    FILE *file;
    long fSize;
    if (!(file = fopen(file_name, "rb"))) {
        perror("Erorr, file can't be opened");
        return NULL;
    }
    fseek(file, 0L, SEEK_END);
    fSize = ftell(file);
    fseek(file, 0L, SEEK_SET);    
    if (fSize < 0) {
        perror("Error, File size is zero.\n");
        fclose (file);
        return NULL;
    }

    const char * ext = get_filename_ext(file_name);

    finfo->size = fSize;
    finfo->type = (char*) malloc(sizeof(char)*(strlen(ext)+1) );
    strcpy(finfo->type, ext);
    
    // allocate memory to contain the whole file:
    finfo->data = (char*) malloc(sizeof(char)*fSize);
    if (!finfo->data) {
        perror("Memory error, can't allocate memory for file");
        fclose(file);
        return NULL;
    }
    // copy the file into the fBuffer:
    size_t read_result = fread (finfo->data, sizeof(char), fSize, file);
    if (read_result != fSize) {
        perror ("Error, can't read file");
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    return finfo;
}


void iterative_file_write(const char *file_name,const  char *data){
    FILE *file;
    if (!(file = fopen(file_name, "wa"))) {
        perror("Erorr, file can't be opened");
        return;
    }

}

void free_file_data(struct FileInfo* finfo){
    free(finfo->type);
    printf("type\n");
    free(finfo->data);
    printf("data\n");
    free(finfo);
    printf("info\n");
}
