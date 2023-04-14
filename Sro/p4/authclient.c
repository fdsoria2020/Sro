#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <err.h>
#include "tools.h"

enum {
    // MAX_BUFF = 100,
    PORT = 9999,
    MAX_BUFF = 284,
    MAX_USER = 256,
    MAX_SHA1 = 20,
};

#define SERVER_ADDRESS "127.0.0.1"    /* server IP */

 
void
get_requirements(uint64_t nonce)  {
    uint64_t timestamp = time(NULL);
    unsigned char key[] = "3f786850e387550fdab836ed7e6dc881de23001b";
    unsigned char sha1_value[MAX_SHA1];


    // hmac_sha1(nonce, timestamp, key, sha1_value);


}

/* This clients connects, sends a text and disconnects */
int main() 
{ 
    /* Test sequences */
    char buf_tx[MAX_BUFF];      
    char buf_rx[100];                     /* receive buffer */
    uint64_t num;
    int sockfd; 
    struct sockaddr_in servaddr; 
    
    /* Socket creation */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) 
        err(EXIT_FAILURE, "CLIENT: socket creation failed...\n");
    else
        printf("CLIENT: Socket successfully created..\n");
    
    memset(&servaddr, 0, sizeof(servaddr));

    /* assign IP, PORT */
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr( SERVER_ADDRESS ); 
    servaddr.sin_port = htons(PORT); 
  
    /* try to connect the client socket to server socket */
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) 
        err(EXIT_FAILURE, "Connection with the server failed...\n");
    
    printf("connected to the server..\n"); 
  
    /* send test sequences*/
    read(sockfd, &num, sizeof(num));
    printf("CLIENT:Received: %lu \n", num);
    // sleep(40);
    uint64_t timestamp = time(NULL);
    unsigned char key[] = "3f786850e387550fdab836ed7e6dc881de23001b";
    unsigned char sha1_value[MAX_SHA1];
    char user[MAX_USER] = "pepe";
    unsigned char message[2*sizeof(uint64_t)];


    memcpy(message, &num, sizeof(num));
	memcpy(message + sizeof(num), &timestamp, sizeof(timestamp));
    printf("Message despues de memcpy vale: ");
	for(int i=15; i>=0; i--) {
        printf("%02x ", message[i]);
    }
    printf("\n");
    // hmac_sha1(num, timestamp, key, sha1_value);
    // hmac_sha1(message, size_unsigned(message), key, sha1_value);
    hmac_sha1(message, 16, key, sha1_value);


    // get_requirements(num);
    memcpy(buf_tx, sha1_value, MAX_SHA1);
    memcpy(buf_tx + MAX_SHA1, &timestamp, sizeof(timestamp));
    memcpy(buf_tx + MAX_SHA1 + sizeof(timestamp), user, MAX_USER);
    write(sockfd, buf_tx, sizeof(buf_tx));     
    read(sockfd, buf_rx, sizeof(buf_rx));
    printf("%s", buf_rx);
       
    /* close the socket */
    close(sockfd); 
} 