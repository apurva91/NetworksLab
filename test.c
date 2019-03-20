#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
// #include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/time.h> 
#include <sys/types.h> 
#include <errno.h> 


#define OUTPUT_BUFFER_SIZE 1024
#define MAX_PENDING_CONN 30
#define MAX_CLIENTS 30


bool isNumber(char str[]){
	int i=0;
	if(str[0]=='-'){
		i=1;
	}
	for(;str[i]!='\0';i++){
		if(!isdigit(str[i])) return false;
	}
	return true;
}

int str_to_pnum(char str[]){
	if(!isNumber(str)){
		return -1;
	}
	int sign, i=0;	
	if(str[0]=='-'){  // Handle negative integers
		sign = -1;
	}
	if(sign==-1){  // Set starting position to convert
		i = 1;
	}
	int n = 0;
	for(; str[i]!='\0'; i++) {
		n = n * 10 + str[i] - '0';
	}
	if(sign==-1){
		return -1;
	}
	if(n>65535){
		return -1;
	}
	return n;
}


int main(int argc,char *argv[]) 
{ 
	if(argc != 2){
		printf("Missing/Extra Arguments provided.\n");
		printf("Usage:- ./a.out <Server Port>\n");
		return -1;
	}
	//checking for whether correct number of arguments are provided
	int port = str_to_pnum(argv[1]);
	if(port==-1){
		printf("Invalid Port Number Must be in range [0-65535]");
	}
	//checking for port number

	struct sockaddr_in address; 
    //Initializing Socket Addresses for the server, structure has 4 attrs:- family, port, address, zero(for padding the data)
	int addrlen = sizeof(address); 

	int server_socket=0;
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("Socket creation error \n"); 
		return -1; 
	} 
    //Initializing the socket, The socket function creates an unbound socket and returns its file descriptor. SOCK_STREAM provides sequenced and reliable data transfer


	int opt = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){ 
		perror("Unable to start the server socket with required options for server"); 
		return -1; 
	}
	// SO_REUSEPORT and SO_REUSEADDR permits to reuse same port and addr respectively for multiple sockets.

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons(port); 

	if(bind(server_socket,(struct sockaddr *)&address,addrlen)<0){ 
		perror("Binding of the server to the port failed"); 
		return -1; 
	} 
	// Binding the server socket to the port passed in command line.

	fd_set socket_fds;
	//set of scoket file descriptors
	
	int client_sockets[MAX_CLIENTS] = {0};
	//initalizing client socket file descriptors.
	
	if (listen(server_socket, MAX_PENDING_CONN) < 0) { 
		perror("Error while listening"); 
		return -1; 
	}

	int max_fd;
	int fd;
	int valread;
	char buffer[OUTPUT_BUFFER_SIZE] = {0}; 

	while(1){
		FD_ZERO(&socket_fds);
		//clear all the fds
		FD_SET(server_socket,&socket_fds);
		//adding the server's socket to the set
		max_fd = server_socket;
		for(int i=0; i<MAX_CLIENTS; i++){
			fd = client_sockets[i];
			if(fd>0){
				FD_SET(fd,&socket_fds);
			}
			//if the file descriptor is valid then add it to the set
			if(fd>max_fd){
				max_fd = fd;
			}
			//max_fd will hold the maximum of all fds it is required for select function as mentioned in the documentation
		}
		int activity = select(max_fd+1,&socket_fds,NULL,NULL,NULL); 
		if(activity<0&&errno!=EINTR){
			perror("Caught a signal\n");
		}
		//Applying the select function to start the monitor on the file descriptors

		if(FD_ISSET(server_socket,&socket_fds)){
			int client_socket = 0; 
			if ((client_socket = accept(server_socket, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0){ 
				perror("Unable to accept the socket"); 
				return -1; 
			} 

			printf("New connection from %s:%d stored in file descriptor of id %d\n",inet_ntoa(address.sin_addr),ntohs(address.sin_port),client_socket); 
			//inform user of socket number - used in send and receive commands 
			// valread = read(client_socket,buffer,1024); 
			// printf("Message from %d: %s\n",client_socket,buffer); 
			char *message = "Hello from server"; 
			if(send(client_socket , message , strlen(message) , 0 )){
				printf("Message sent at %s:%d\n",inet_ntoa(address.sin_addr),ntohs(address.sin_port));
			} 
			printf("Hello sent from the server\n");
			for(int i=0; i<MAX_CLIENTS; i++){
				if(client_sockets[i]==0){
					client_sockets[i]=client_socket;
					printf("Adding %d to list of client sockets\n",client_socket);
					break; 
				}
			}
			printf("ok");
			//adding socket to the current client list
		}
		//Something has changed in the master socket, means a new connection has come, so we should serve it

		for(int i=0; i<MAX_CLIENTS; i++){
			fd = client_sockets[i];
			if(FD_ISSET(fd,&socket_fds)){
				if((valread=read(fd,buffer,1024))==0){
					getpeername(fd,(struct sockaddr*)&address,(socklen_t*)&addrlen);
					printf("Host from %s:%d disconnected\n",inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
					close(fd);
					client_sockets[i]=0;
					//removing the socket from client sockets list
				}//the socket corresponding to current fd has been disconnected
				else{
					buffer[valread] = '\0';
					printf("%s\n", buffer);
					send(fd,buffer,strlen(buffer),0); 
				}
			}
		}
		//Check for all other clients if their file descriptor has changed
	}

	return 0; 

}  