#include <stdio.h> 
#include <string.h> //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> //close 
#include <arpa/inet.h> //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
	
#define TRUE 1 
#define FALSE 0 
#define PORT 8888 
	
int main(int argc , char *argv[]){

	char buffer[OUTPUT_BUFFER_SIZE] = {0}; 
	int valread = read(client_socket,buffer,1024); 
	printf("%s\n",buffer );

	char key[10] = {'1','0','0','0','0','0','1','1','1'};
	key[9] = '\0';

	bool err = isErrorFree(buffer,key,strlen(buffer),strlen(key));
	printf("%d\n",err);


	char *message = "Hello from server"; 
	send(client_socket , message , strlen(message) , 0 ); 
	printf("Hello message sent\n"); 

	return 0; 
} 
