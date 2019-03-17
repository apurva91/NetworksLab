#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
// #include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <unistd.h>

#define PORT 12345 
#define OUTPUT_BUFFER_SIZE 1024

int main(int argc,char const *argv[]) 
{ 
	struct sockaddr_in client_address, server_address; 
    //Initializing Socket Addresses for client and server structure has 4 attrs:- family, port, address, zero(for padding the data)

	char *message = "Hello from client"; 
    //The message to be sent

	int sock = 0;

	if ((sock = socket(AF_INET,SOCK_STREAM,0)) < 0){ 
		printf("\nSocket creation error \n"); 
		return -1; 
	}
    //Initializing the socket, The socket function creates an unbound socket and returns its file descriptor. SOCK_STREAM provides sequenced and reliable data transfer

	memset(&server_address,'0',sizeof(server_address)); 
   	// filling server_address struct to all 0's

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT); 
    // htons:- host to network short, converts the given port number to hexa decimal depending on what machine type is it, big endian or little endian


	if(inet_pton(AF_INET,"127.0.0.1",&server_address.sin_addr) <= 0){ 
		printf("\nInvalid server address / Address not supported \n"); 
		return -1; 
	} 
    // Check if the given IP address belongs to the given family and convert it to binary and store it in server_address

	if (connect(sock,(struct sockaddr *)&server_address,sizeof(server_address)) < 0){ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}
    // coverting sockaddr_in(socket for IP based communication) to sockaddr(generic socket)
    // Check if connection can be initiated, if yes returns 0, else -1 

	send(sock,message,strlen(message),0); 
    //send the message, the 0 as last parameter is the field for flags.
	printf("Message Sent Successfully\n"); 

	char buffer[OUTPUT_BUFFER_SIZE] = {0};
    //initalizing output buffer

	int valread = read(sock,buffer,OUTPUT_BUFFER_SIZE); 
	printf("%s\n",buffer); 

	return 0; 
} 