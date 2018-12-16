#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <pthread.h>
#include <math.h>
#include "../lib/fileHandler.h"
#include "../lib/HttpMessage.h"
#include "../lib/stringFuncs.h"
#include <queue>

#define DEFAULT_PORT 80
#define THREADS_NUMBER 1
#define HOST "127.0.0.1"
#define BUFFER_SIZE 1024
#define CLIENT_PATH "client/received"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

long min(long x, long y){
    if(y>x)
        return x;
    else
        return y;
}

struct thread_args {
    int id;
    long port_number;
};

struct FileInfo* read_request_file(char *file_name){
    struct FileInfo* info = file_read(file_name);
    return info;
}


void send_data(int socketID, char * data,int size)
{
    pthread_mutex_lock(&lock);
    send(socketID, data, size, 0);
    pthread_mutex_unlock(&lock);
        /* Debug print. */
    // printf("theree\n");
    // printf("Data sent (%ld):\n'%s'\n", strlen(data), data);
}


void post_request(int clientSocket, char* request){
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    // send request
    send(clientSocket, request, strlen(request), 0);
    // recive validation signal
    size_t reply_data_size = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    char* msg = buffer + strlen(buffer)-4;
    if(reply_data_size < 1){
        printf("No verification signal received, can't send file\n");

    } else if(strncmp("Ok", msg, 2) == 0){
        // get url
        char url[100];
        char* pointer = request;
        while(isalpha(*pointer)) pointer++;
        pointer++;
        
        int i=0;
        while(!isspace((char)*pointer)){
            url[i++] = *pointer;
            pointer++;
        }
        url[i] = 0;
        // read url file
        struct FileInfo* finfo = file_read(url+1);
        char siz_info[15];
        sprintf(siz_info, "%ld", finfo->size);
        // send file size
        send(clientSocket, siz_info, strlen(siz_info), 0);
        // send file data
        char* data =  (char*) malloc(BUFFER_SIZE);
        long sent_data = 0;
        pointer = finfo->data;
        while(sent_data < finfo->size){
            // clear old data
            memset(data, 0, BUFFER_SIZE);
            // advandce data pointer
            //min((finfo->size -(pointer-finfo->data)), BUFFER_SIZE-1);
            long size = min((finfo->size -(pointer-finfo->data)), BUFFER_SIZE-1);
            printf("%d\n", size);
            memcpy(data, pointer, size);
            
            pointer += size;
            sent_data += size;
            // send required file
            send_data(clientSocket, data,size);
        }
        free(data);

    } else{
        printf("Can't recognize verification signal, file can't be sent\n");
    }
}


void get_request(int clientSocket, char* request){
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int receive_length = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    // get url
    char url[100];
    char* pointer = request;
    while(isalpha(*pointer)) pointer++;
    pointer++;
    
    int i=0;
    while(!isspace((char)*pointer)){
        url[i++] = *pointer;
        pointer++;
    }
    url[i] = 0;
    // get file size
    char info_buff[15];
    i = 0;
    pointer = strchr(buffer, ':') + 2;
    while(*pointer != '\r'){
        info_buff[i++] = pointer[0];
        pointer++;
    }
    info_buff[i] = 0;
    long file_size = atol(info_buff);
    
    // advance pointer to body start
    pointer += 2;
    bool has_chars = false;
    while(true){
        if(*pointer == '\n'){
            if(has_chars){
                pointer++;
                has_chars = false;
            }else{
                pointer++;
                break;
            }
        }
        if(isalpha(*pointer))
            has_chars = true;
            
        pointer++;
    }
    // write received data to file
    size_t received_data = receive_length-(pointer -buffer);
    char file_name[100];
    sprintf(file_name, "%s%s", CLIENT_PATH,strrchr(url, '/'));
    FILE *received_file;
    if (!(received_file = fopen(file_name, "wb"))) {
        perror("Erorr, file can't be opened");
        return;
    }
    fwrite(pointer, sizeof(char), received_data, received_file);
    // printf("Data received (%d):\n%s\n", received_data, buffer);
    while(received_data < file_size){
        memset(buffer, 0, BUFFER_SIZE);
        size_t received = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        received_data += received;
        fwrite(buffer, sizeof(char), received, received_file);
        // printf("Data received (%d):\n%s\n", received_data, buffer);
    }
    fclose(received_file);
}

void * client_thread(void *arg){
    std::queue <char*> queue_request;
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;
    // Create the socket. 
    if ((clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
      printf("Socket creation failed!\n");
    else {
        //Configure settings of the server address
        serverAddr.sin_family = AF_INET;
        //Set port number, using htons function
        struct thread_args* args = (struct thread_args*) arg;
        printf("%ld\n", args->port_number);
        serverAddr.sin_port = htons(args->port_number);
        //Set IP address to the host
        serverAddr.sin_addr.s_addr = inet_addr(HOST);
        memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
        //Connect the socket to the server using the address
        addr_size = sizeof(serverAddr);
        if (connect(clientSocket, (struct sockaddr *) &serverAddr,addr_size) < 0)
            printf("Connection failed !!\n");
        else {
            printf("Connected..........\n");
           // struct FileInfo* finfo = read_request_file("client/request_longtext.txt");

            // if (strncmp("GET", finfo->data, 3) == 0)
            //     get_request(clientSocket, finfo->data);
            // else if(strncmp("POST", finfo->data, 4) == 0)
            //     post_request(clientSocket, finfo->data);
            // else
            //     printf("Erorr, unsupported command !\n");
            

            // handle file of multiple requests
            struct FileInfo* finfo = read_request_file("client/multiple_requests.txt");
            char line[200];
            int i;
            char * pointer = finfo->data;
            printf("read\n");
            while(*pointer != '\0'){
                i = 0;
                while((*pointer != '\r' && *pointer!= '\n' && *pointer!='\0')){
                    line[i++] = pointer[0];
                    // printf("%c\n", pointer[0]);
                    pointer++;
                }
                line[i] = 0;

                printf("%s\n", line);
                if (strncmp("GET", line, 3) == 0){
                    char * request = (char *)malloc(strlen(line));
                    strcpy(request, line);
                     queue_request.push(request);
                     send(clientSocket, request, strlen(request), 0);
                     //sleep(1);
                     // get_request(clientSocket, line);
                }
                else if(strncmp("POST", line, 4) == 0)
                    post_request(clientSocket, line);
                else
                    printf("Erorr, unsupported command !\n");
                if(*pointer =='\0') break;
                pointer+=2; // move pointer past <CR> OR <LF>
                // skipe spaces
                while(*pointer == ' ' || *pointer == '\t') pointer++;
                
            }

            while(!queue_request.empty()){
                char* line =queue_request.front();
                printf("handling recieve%s\n",line );
                queue_request.pop();
                get_request(clientSocket, line);
                free(line);
            }
        }

    }
    printf("finished\n");

    // close socket and exit thread
    //sleep(2);
    //close(clientSocket);
    printf("Client Socket is closed  %d\n",clientSocket );
    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    // handle arguments
    long port_number = DEFAULT_PORT;
    if(argc > 2){
        printf("Too many arguments, at maximum one argument is expected: {port number}\n");
        return 1;
    } else if(argc > 1)
        port_number = atol(argv[1]);

    struct thread_args ta;
    ta.port_number = port_number;
    client_thread(&ta);
    // //client_thread(&ta);
    // pthread_t tid[THREADS_NUMBER];
    // int i =0 ;
    // while(i < THREADS_NUMBER)
    // {
    //     if( pthread_create(&tid[i], NULL, client_thread, &ta) != 0 )
    //            printf("Failed to create thread\n");
    //     i++;
    // }
    // sleep(1);
    // while(i>0)
    // {
    //   pthread_join(tid[--i],NULL);
    //    printf("%d:\n",i);
    // }
    return 0;
}
