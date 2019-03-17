#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
// #include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>

#define OUTPUT_BUFFER_SIZE 1024
#define MAX_PENDING_CONN 3

char subs[100];

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

	struct sockaddr_in server_address; 
    //Initializing Socket Addresses for the server, structure has 4 attrs:- family, port, address, zero(for padding the data)
	int addrlen = sizeof(server_address); 

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

	server_address.sin_family = AF_INET; 
	server_address.sin_addr.s_addr = INADDR_ANY; 
	server_address.sin_port = htons(port); 

	if(bind(server_socket,(struct sockaddr *)&server_address,addrlen)<0){ 
		perror("Binding of the server to the port failed"); 
		return -1; 
	} 
	// Binding the server socket to the port passed in command line.

	if (listen(server_socket, MAX_PENDING_CONN) < 0) { 
		perror("Error while listening"); 
		return -1; 
	}
	int  client_socket; 
	if ((client_socket = accept(server_socket, (struct sockaddr *)&server_address,(socklen_t*)&addrlen))<0){ 
		perror("accept"); 
		return -1; 
	} 

	char buffer[OUTPUT_BUFFER_SIZE] = {0}; 
	int valread = read(client_socket,buffer,1024); 
	printf("%s\n",buffer ); 

	char *message = "Hello from server"; 
	send(client_socket , message , strlen(message) , 0 ); 
	printf("Hello message sent\n"); 

	return 0; 

} 