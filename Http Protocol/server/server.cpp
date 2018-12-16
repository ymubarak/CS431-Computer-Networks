#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<fcntl.h> // for open
#include<unistd.h> // for close
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include "../lib/HttpMessage.h"
#include "requestHandler.h"
#include <math.h>
#include <queue>

#define DEFAULT_PORT 80
#define LOCAL_HOST "127.0.0.1"
#define MAX_CONNECTIONS 50
#define REQUEST_SIZE 2000
#define INIT_WAIT 5
   
long number_of_connections = 0;
int min_time = 2;
struct timeval  timeout = {INIT_WAIT,0};
/* Setting time out vlaues for some seconds */
// timeout.tv_sec = INIT_WAIT;   // WAIT seconds
// timeout.tv_usec = 0;    // 0 milliseconds

void update_timeout(){
    long extra = number_of_connections<2 ? INIT_WAIT-min_time: INIT_WAIT/sqrt(number_of_connections);
    timeout.tv_sec = (int) (min_time + extra);
   // printf("timeout updated to : %d\n", timeout.tv_sec);
    // e^(-number_of_connections)
    // 1/ sqrt(number_of_connections)
}

struct thread_args {
    int id;
    long port_number;
};

void * socket_thread(void *arg)
{
    char client_message[REQUEST_SIZE];
    int socketID=*((int *)arg);
   // struct thread_args* args = (struct thread_args*) arg;
    //int socketID = args->id;
    printf("socketID     %d     \n", socketID);
    fd_set input_set;
    FD_ZERO(&input_set);
    FD_SET(socketID, &input_set);
    std::queue <Request *> request_queue; 
    while(true)
    {   
        int ready, buffer_out;
        update_timeout();
        ready = select(FD_SETSIZE, &input_set, NULL, NULL, &timeout);
        // printf("found req: timeout becomes: %d\n", timeout.tv_sec);

        if (ready < 0){
           printf("Error: Bad file descriptor set.\n");
           break;
        }
        else if (ready == 0){
           printf("Error: Packet timeout expired.\n");
           break;
        }
        else{
            // new packet received
            size_t read_size;
            do{
                read_size = read(socketID, client_message, REQUEST_SIZE);
            }while(read_size<0);
            if (read_size==0)               // connection closed ;
                break;           

           // printf("request Size :%d\n",read_size );
            struct Request *req = parse_request(client_message);
           // printf(" request recived: %s\n", req->url);
            // print_request(req);
            if(req->method == Method::GET){
                //in queue
                request_queue.push(req);
            }
            else if(req->method == Method::POST){
                handle_post_request(socketID, req);
                free(req);
            }else{
                printf("Unsupported request\n");
            }
            
        }
    }
    while(!request_queue.empty()){
        struct Request *req = request_queue.front();
        request_queue.pop();
        printf("sending    :%s\n",req->url );
        handle_get_request(socketID, req);
        free(req);
    }

    close(socketID);
    // decrease the number of connections as the connection closes
    number_of_connections--;
    //update_timeout();
    printf("Exit socket thread  %d \n",socketID);
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
    
    int serverSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    //Create the socket. 
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    // Configure settings of the server address struct
    // Address family = Internet 
    serverAddr.sin_family = AF_INET;
    // set port number, using htons function to use proper byte order 
    serverAddr.sin_port = htons(port_number);
    //Set IP address to localhost 
    serverAddr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    //Set all bits of the padding field to 0 
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    //Bind the address struct to the socket 
    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    //Listen on the socket, with N max connection requests queued 
    if(listen(serverSocket, MAX_CONNECTIONS) == 0)
        printf("Listening\n");
    else
        printf("Error, can't listen to port\n");
    
    pthread_t tid[60];
    int i = 0;
    int sockets [MAX_CONNECTIONS];
    while(true)
    {
        //Accept call creates a new socket for the incoming connection
        if( i > MAX_CONNECTIONS)
        {
          i = 0;
          while(i < MAX_CONNECTIONS)
          {
            pthread_join(tid[i++],NULL);
          }
          i = 0;
        }
        addr_size = sizeof(serverStorage);
        int newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);
        sockets[number_of_connections]=newSocket;
        printf("%d\n", newSocket);
        //newSocket=0;
        number_of_connections++;
        // update_timeout();
        // for each client request, create a thread and assign the client request to it
        // By doing so, the main thread can entertain next requests.
        if(pthread_create(&tid[i++], NULL, socket_thread, &newSocket) != 0 )
           printf("Failed to create thread\n");

    }
    return 0;
}
