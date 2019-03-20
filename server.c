#include <stdio.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <stdlib.h> 
// #include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>

#define OUTPUT_BUFFER_SIZE 1024
#define MAX_PENDING_CONN 50
#define RTT 0.5

const char * IP_ADDRESS = "0.0.0.0";
char client_message[2000];
char buffer[OUTPUT_BUFFER_SIZE];
int valread;

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

void printnew(int socketfd){
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    getpeername(socketfd,(struct sockaddr*)&address,(socklen_t*)&addrlen);
    printf("New client connected from %s:%d\n",inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
}

void printmessage(int socketfd, char message[]){
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    getpeername(socketfd,(struct sockaddr*)&address,(socklen_t*)&addrlen);
    printf("Message from %s:%d : %s\n",inet_ntoa(address.sin_addr) , ntohs(address.sin_port),message);
}

void printdisconnect(int socketfd){
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    getpeername(socketfd,(struct sockaddr*)&address,(socklen_t*)&addrlen);
    printf("Host from %s:%d disconnected\n",inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
}

void * socketThread(void *arg){
    int newSocket = *((int *)arg);  
    while(1){   
        if((valread=read(newSocket,buffer,1024))==0){
            break;
        }
        else{
            if(strlen(buffer)==0){
                sleep(RTT);
                continue;
            }
            else{
                send(newSocket,"Hello Client\n",strlen("Hello Client\n"),0);
                printmessage(newSocket,buffer);
                sleep(RTT);
            }
        }
    }
    printdisconnect(newSocket);
    close(newSocket);
    pthread_exit(NULL);
}

int main(int argc,char *argv[]){
    if(argc != 2){
        printf("Missing/Extra Arguments provided.\n");
        printf("Usage:- ./<output_file> <Server Port>\n");
        return -1;
    }
    //checking for whether correct number of arguments are provided
    int port = str_to_pnum(argv[1]);
    if(port==-1){
        printf("Invalid Port Number Must be in range [0-65535]\n");
        return -1;
    }
    //checking for port number
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    //Initializing the socket, The socket function creates an unbound socket and returns its file descriptor. SOCK_STREAM provides sequenced and reliable data transfer

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    // htons:- host to network short, converts the given port number to hexa decimal depending on what machine type is it, big endian or little endian

    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    // filling server_address struct to all 0's

    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    // Binding the server socket to the port passed in command line.
    //Listen on the socket, with 40 max connection requests queued 

    if(listen(serverSocket,MAX_PENDING_CONN)==0){
        printf("Listening\n");
    }
    else{
        printf("Error\n");
    }

    pthread_t tid[60];

    int i = 0;

    while(1){
    //Accept call creates a new socket for the incoming connection
        addr_size = sizeof(serverStorage);
        newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);
        //for each client request creates a thread and assign the client request to it to process
        //so the main thread can entertain next request
        printnew(newSocket);
        if(pthread_create(&tid[i],NULL,socketThread,&newSocket)!=0){
            printf("Failed to create thread\n");
        }
        if(i>=MAX_PENDING_CONN){
            i = 0;
            while(i<MAX_PENDING_CONN){
                pthread_join(tid[i++],NULL);
            }
            i = 0;
        }
    }
    return 0;
}