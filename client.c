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

void crc(char *input,char *key,char *result,int keylen,int msglen){
	char key1[30],temp[30],quot[100],rem[30];
	int i,j;

	strcpy(key1,key);
	for(i=0;i<keylen-1;i++)
	{
		input[msglen+i]='0';
	}
	for(i=0;i<keylen;i++)
		temp[i]=input[i];
	for(i=0;i<msglen;i++)
	{
		quot[i]=temp[0];
		if(quot[i]=='0')
			for(j=0;j<keylen;j++)
				key[j]='0';
			else{
				for(j=0;j<keylen;j++)
					key[j]=key1[j];
			}
			for(j=keylen-1;j>0;j--)
			{
				if(temp[j]==key[j])
					rem[j-1]='0';
				else
					rem[j-1]='1';
			}
			rem[keylen-1]=input[i+keylen];
			strcpy(temp,rem);
		}
		strcpy(rem,temp);
		printf("\nQuotient is ");
		for(i=0;i<msglen;i++)
			printf("%c",quot[i]);
		printf("\nRemainder is ");
		for(i=0;i<keylen-1;i++)
			printf("%c",rem[i]);
		printf("\nFinal data is: ");
		for(i=0;i<msglen;i++){
			result[i]=input[i];
		}
		for(i=0;i<keylen-1;i++){
			result[msglen+i] = rem[i];
		}
	}
/*
BER -> Bit Error Rate = No of bits to have error in a bit transmitted on an average
So we randomly choose these bits and generate error
*/
void randomError(float ber,char *pure,char *noisy,int size){
	int n = floor(ber*size);

	int indx[n];
	for(int i=0;i<n;i++){
		indx[i] = rand() % size-1;
		printf("%d\n",indx[i]);
	}
	printf("\n");
	for(int i=0;i<n;i++){
		if(pure[indx[i]]) noisy[indx[i]] = '0';
		else noisy[indx[i]] = '1';
	}
}

int main(int argc,char *argv[]) 
{ 
	if(argc != 3){
		perror("Missing/Extra Arguments provided.\n");
		perror("Usage:- ./a.out <Server IP Address> <Server Port>\n");
		return -1;
	}
	//checking for whether correct number of arguments are provided
	int port = str_to_pnum(argv[2]);
	if(port==-1){
		perror("Invalid Port Number Must be in range [0-65535]");
	}
	//checking for port number

	struct sockaddr_in client_address, server_address; 
    //Initializing Socket Addresses for client and server structure has 4 attrs:- family, port, address, zero(for padding the data)

	
	char *message = "Hello from client"; 
    //The message to be sent
	int i,j,keylen,msglen;
	char input[100],temp[30],quot[100],rem[30],key1[30];
	printf("Enter Data: ");
	gets(input);

	//CRC-8 Generator Polynomial
	char key[10] = {'1','0','0','0','0','0','1','1','1'};
	key[9] = '\0';
	
	keylen=strlen(key);
	msglen=strlen(input);

	char result[keylen+msglen-1];
	crc(input,key,result,keylen,msglen);

	for(i=0;i<msglen+keylen-1;i++){
		printf("%c",result[i]);			
	}
	

	int sock = 0;

	if ((sock = socket(AF_INET,SOCK_STREAM,0)) < 0){ 
		perror("Socket creation error \n"); 
		return -1; 
	}
    //Initializing the socket, The socket function creates an unbound socket and returns its file descriptor. SOCK_STREAM provides sequenced and reliable data transfer

	memset(&server_address,'0',sizeof(server_address)); 
   	// filling server_address struct to all 0's

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port); 
    // htons:- host to network short, converts the given port number to hexa decimal depending on what machine type is it, big endian or little endian


	if(inet_pton(AF_INET,argv[1],&server_address.sin_addr) <= 0){ 
		perror("Invalid server address / Address not supported \n"); 
		return -1; 
	} 
    // Check if the given IP address belongs to the given family and convert it to binary and store it in server_address

	if (connect(sock,(struct sockaddr *)&server_address,sizeof(server_address)) < 0){ 
		perror("Connection Failed \n"); 
		return -1; 
	}
    // coverting sockaddr_in(socket for IP based communication) to sockaddr(generic socket)
    // Check if connection can be initiated, if yes returns 0, else -1 

	send(sock,result,msglen+keylen-1,0); 

    //send the message, the 0 as last parameter is the field for flags.
	printf("Message Sent Successfully\n"); 

	char buffer[OUTPUT_BUFFER_SIZE] = {0};
    //initalizing output buffer

	int valread = read(sock,buffer,OUTPUT_BUFFER_SIZE); 
	printf("%s\n",buffer); 

	// send(sock,message,strlen(message),0); 
	// printf("Message Sent Successfully\n");
	// valread = read(sock,buffer,OUTPUT_BUFFER_SIZE); 
	// printf("%s\n",buffer); 
	// while(1){}
	return 0; 
} 