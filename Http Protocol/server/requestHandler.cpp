#include<stdlib.h>
#include<stdio.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<string.h>
#include<string>
#include<pthread.h>
#include<math.h>
#include "requestHandler.h"
#include "../lib/fileHandler.h"

#define STATUS_LINE_LEN 30
#define HEADERS_LEN 200
#define BUFFER_SIZE 1024

#define SERVER_PATH "server/received"

void send_data(int socketID, char * data);
void send_data(int socketID, char * data, int size_sent);

long min(long x, long y){
    if(y>x)
        return x;
    else
        return y;
}

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


void handle_post_request(int socketID, struct Request* req){
    char* httpVersion;
    char* msg = "Ok"; 
    int status;

    httpVersion = req->version;
    status = StatusCode::OK;
    // set status line data
    char * status_line = (char*) malloc (STATUS_LINE_LEN);
    sprintf(status_line, "%s %d %s\r\n", httpVersion, status, msg);
    // send verification signal
    send_data(socketID, status_line);
    // receive file size
    char size_info[15];
    recv(socketID, size_info, 15, 0);
    long file_size = atol(size_info);
    printf("size  :%ld\n",file_size );
    // receive data
    char file_name[100];
    sprintf(file_name, "%s%s", SERVER_PATH, strrchr(req->url, '/'));
    FILE *received_file;
    if (!(received_file = fopen(file_name, "wb"))) {
        perror("Erorr, file can't be opened");
        return;
    }
        

    char buffer[BUFFER_SIZE];
    size_t received_data = 0;
    while(received_data < file_size){
        memset(buffer, 0, BUFFER_SIZE);
        size_t received = recv(socketID, buffer, BUFFER_SIZE, 0);
        received_data += received;
        fwrite(buffer, sizeof(char), received, received_file);
        printf("Data received (%d):\n%s\n",received, buffer);
    }
    fclose(received_file);
}

void handle_get_request(int socketID, struct Request* req){
    char* httpVersion;
    bool err;
    int status;
    struct FileInfo* finfo = NULL;

    httpVersion = req->version;
    if(!file_exists(req->url+1)){
        status = StatusCode::NOTFOUND;
        err = true;
    }else
    {
        status = StatusCode::OK;
        err = false;
        finfo = file_read(req->url+1);
    }
    // set status line data
    char * status_line = (char*) malloc (STATUS_LINE_LEN);
    if(err)
        sprintf(status_line, "%s %d %s\r\n", httpVersion, status, "NotFound");
    else
        sprintf(status_line, "%s %d %s\r\n", httpVersion, status, "Ok");

    if(finfo == NULL){ // file not found
        send_data(socketID, status_line);
        free(status_line);
        return;
    }
    // file exists
    char* headers = (char*) malloc (HEADERS_LEN);
    sprintf(headers,
        "content-length: %ld\r\n"
        "content-type: %s\r\n"
        "\r\n", finfo->size, finfo->type);

    // send status line, headers and the rest of buffer occupied with part of data
    char* data =  (char*) malloc(BUFFER_SIZE);
    memset(data,0,BUFFER_SIZE);
    strcpy(data, status_line);
    strcat(data, headers);
    long remaining_buffer;
    remaining_buffer = BUFFER_SIZE - strlen(status_line) - strlen(headers) - 1;
    memcpy(data + strlen(status_line) + strlen(headers) ,finfo->data,remaining_buffer);
    // strncat(data, finfo->data, remaining_buffer);

    // printf("Size   : %d\n",finfo->size);
    int size_sent= remaining_buffer <= finfo->size? BUFFER_SIZE-1:strlen(status_line) + strlen(headers)+finfo->size;
    send_data(socketID, data,size_sent);

    // send the rest of file, (if any) are remained
    long sent_data = remaining_buffer;
    char* pointer = finfo->data + sent_data;
    while(sent_data < finfo->size){
        // clear old data
        memset(data, 0, BUFFER_SIZE);
        // advandce data pointer
        long size = min((finfo->size -(pointer-finfo->data)), BUFFER_SIZE-1);
        // long size = min(strlen(pointer), BUFFER_SIZE-1);
        memcpy(data, pointer, size);
        pointer += size;
        sent_data += size;
        // send required file
        send_data(socketID, data,size);
    }
    // free memory
    free(status_line);
    free(data);
    // free_file_data(finfo);
}


void send_data(int socketID, char * data)
{
    pthread_mutex_lock(&lock);
    send(socketID, data, strlen(data), 0);
    pthread_mutex_unlock(&lock);
        /* Debug print. */
    printf("theree\n");
    printf("Data sent (%ld):\n'%s'\n", strlen(data), data);
}
void send_data(int socketID, char * data,int size)
{
    pthread_mutex_lock(&lock);
    send(socketID, data, size, 0);
    pthread_mutex_unlock(&lock);
        /* Debug print. */
    // printf("Data sent (%ld):\n'%s'\n", strlen(data), data);
}
