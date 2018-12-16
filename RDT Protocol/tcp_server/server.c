#include "../lib/tcp.h"
#include "../lib/fileHandler.h"
#define PATH "tcp_server/"

ssize_t tcp_send(int sockfd, const void *buf, size_t len, int flags){
	//return sr_send(sockfd, buf, len,flags);
	return sw_send(sockfd, buf, len,flags);
	//return gbn_send(sockfd, buf, len,flags);

}
ssize_t tcp_recv(int sockfd, void *buf, size_t len, int flags){
	//return sr_recv(sockfd, buf, len,flags);
	return sw_recv(sockfd, buf, len,flags);
	//return gbn_recv(sockfd, buf, len,flags);	
}

struct server_input {
    int port, max_window_size;
    float seed, loss_prob;
};

struct server_input* read_server_file(char *file_name){
    struct FileInfo* finfo = file_read(file_name);
    struct server_input* ser_in = malloc(sizeof(struct server_input));
    // parse file content
    char* pointer = finfo->content;
    char buffer[50];
    // set serve port
    int i=0;
    while(!isspace((char)*pointer)){
        buffer[i++] = *pointer;
        pointer++;
    }
    buffer[i] = 0;
    ser_in->port = atoi(buffer);
    // printf("%d\n", ser_in->port);

    // set max window size
    i=0;
    pointer++;
    while(!isspace((char)*pointer)){
        buffer[i++] = *pointer;
        pointer++;
    }
    buffer[i] = 0;
    ser_in->max_window_size = atoi(buffer);
    // printf("%d\n", ser_in->max_window_size);

    // set random seed
    i=0;
    pointer++;
    while(!isspace((char)*pointer)){
        buffer[i++] = *pointer;
        pointer++;
    }
    buffer[i] = 0;
    ser_in->seed = atof(buffer);
    // printf("%f\n", ser_in->seed);

    // set max window size
    i=0;
    pointer++;
    while(!isspace((char)*pointer) && *pointer != '\0'){
        buffer[i++] = *pointer;
        pointer++;
    }
    buffer[i] = 0;
    ser_in->loss_prob = atof(buffer);
    // printf("%f\n", ser_in->loss_prob);

    return ser_in;
}


int main(int argc, char *argv[])
{
	int sockfd;
	int newSockfd;
	int numRead;
	struct sockaddr_in server;
	struct sockaddr_in client;
	FILE *file;     /* input file pointer */
	socklen_t socklen;
	char buf[DATALEN * N];   /* buffer to send packets */
	
	/*----- reading file specs -----*/
	struct server_input* input = read_server_file("tcp_server/server.in");

	/*----- Opening the socket -----*/
	if ((sockfd = tcp_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		perror("Error! Server failed to create tcp_socket");
		exit(-1);
	}
	
	printf("Random Generator Seed: %.2f\n", (input->seed));
	srand(input->seed);
	/*--- Setting the server's parameters -----*/
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family      = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	// printf("%s\n",INADDR_ANY );
	printf("Server Port: %d\n", (input->port));
	server.sin_port = htons((input->port));

	printf("server max window_size: %d\n", input->max_window_size);
	s.max_window_size = input->max_window_size;
	
	printf("packet loss probability: %.2f\n", input->loss_prob);
	s.loss_prob = input->loss_prob ;
	
	/*----- Binding to the designated port -----*/
	if (tcp_bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1){
        perror("Error, sever can't bind socket to address!");
		exit(-1);
	}
	
	/*----- Listening to new connections (not impemented ToDo) -----*/
	if (tcp_listen(sockfd, 1) == -1){
		perror("Error, sever fails to be listening");
		exit(-1);
	}

	printf("Listening\n");

	/*----- Waiting for the client to connect -----*/
	socklen = sizeof(struct sockaddr_in);
	newSockfd = tcp_accept(sockfd, (struct sockaddr *)&client, &socklen);
	if (newSockfd == -1){
		perror("Error, server can't accept clinet connection");
		exit(-1);
	}
	
	/*----- Reading from the socket the file name-----*/
	if ((numRead = tcp_recv(newSockfd, buf, DATALEN*MAX_WINDOW_SIZE, 0)) == -1){
			perror("Error, Server failed to recive client request");
			exit(-1);
	}
	printf("Requested File Name: %s\n", buf);
	char fname[100];
	sprintf(fname,"%s%s",PATH, buf);

	if ((file = fopen(fname, "rb")) == NULL){
		perror("Server can't find/open the requested file !");
		exit(-1);
	}

	/* read file */
	fseek(file, 0L, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);
	
	long sent_data = 0;
	int lsize = DATALEN * N;

	printf("Start Sending File....................\n");
	/*----- Reading from the file and sending it through the socket -----*/
	while (true){
		numRead = fread(buf, sizeof(char), lsize, file);
		if (numRead != lsize && ferror(file)){
			perror("Reading Erro");
			exit(-1);	
		}
		if(numRead == 0)
			break;
		if (tcp_send(sockfd, buf, numRead, 0) == -1){
			perror("Error, server can't send packet");
			exit(-1);
		}
		sent_data += numRead;
	}
	printf("Total sent data is %ld out of %ld\n", sent_data, file_size);
	
	/*----- Closing the socket -----*/
	if (tcp_close(sockfd) == -1){
		perror("Server can't close connection");
		exit(-1);
	}

	/*----- Closing the file -----*/
	if (fclose(file) == EOF){
		perror("Error, server can't close file");
		exit(-1);
	}
			
	return (0);
}
