#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>

/* sockets */
#include <netdb.h> 
#include <netinet/in.h> 

/* strings / errors*/
#include <string.h>
#include <err.h>

/*openssl*/
#include <openssl/rand.h>

#include "tools.h"


enum{
    MAX_BUFF = 284,
    MAX_LOGIN = 256,
    MAX_SHA1 = 20,
    PORT = 9999,
};

#define SERVER_ADDRESS "127.0.0.1"    /* server IP */

/*
    Create a nonce using a cryptographically secure pseudo random number generator 
*/
uint64_t 
csprng()
{
   
    uint64_t number;
    if (RAND_bytes((unsigned char*)&number, sizeof(number)) != 1) {
        // Error al generar números aleatorios
        errx(EXIT_FAILURE, "Error creating a nonce");
    }
    return number;
}

void
autenticate(char* autorized_account, char* account)
{
    /**strcspn devuelve la longuitud de la cadena de caracteres hasta encontrar un caracter dado en el segundo puesto*/
    // int i = strcspn(account, ":");
    // printf("Esto hay en account: %c\n", account[i]);
    if (strstr(autorized_account, account) != NULL)
        printf("SUCCESS\n");
    else
        printf("FAILURE\n");

}

unsigned char*
get_key(char* account)
{
    char* token;
    char* aux;
    token = strtok_r(account, ":", &aux);
    token = strtok_r(NULL, ":", &aux);
    return (unsigned char*)token;
}

char *
get_accounts(char* file, char* user, char * buffer)
{
    FILE *fd;
    int found = 0;
    fd = fopen(file, "rb");
    printf("user es %s", user);
    if (fd == NULL)
        err(EXIT_FAILURE, "Cannot open the file");

    while (fgets(buffer, MAX_BUFF, fd) != NULL || !found) {
        if (ferror(fd))  {
            fclose(fd);
            err(EXIT_FAILURE, "Error reading the file");
        }
        buffer[strcspn(buffer, "\r\n")] = '\0';

        if (strstr(buffer, user) != NULL) {
            // printf("Se ha leido: %s\n", buffer);
            found = 1;
        }
    }

    if (fclose(fd) != 0)
        err(EXIT_FAILURE, "Error closing the file");
    
    return buffer;
}

void
check_hmacsha1(unsigned char* r, char *user, char *file, uint64_t timestamp, uint64_t nonce)
{
    char* account;
    // unsigned char key[] = "3f786850e387550fdab836ed7e6dc881de23001b";
    // unsigned char key[] = "clave";
    unsigned char * key;
    char buffer[MAX_BUFF];
    uint64_t result;
    unsigned char message[2*sizeof(uint64_t)];
    // unsigned char message[50] = "estos son los datos";

    printf("El nonce vale: %lu\n", nonce);
    printf("Timestamp: %lu \n", timestamp);
    account = get_accounts(file, user, buffer);
    // printf("EL tamaño de account es: %ld\n", sizeof(account));
    key = get_key(account);
    // printf("La clave es: %s - %ld bytes\n", key, sizeof(*key));

    // result = (nonce << 32) | timestamp;
    // result = nonce || timestamp;
    // printf("El or da: %lu \n", result);

    // memcpy(message, &result, sizeof(result));
    memcpy(message, &nonce, sizeof(nonce));
	memcpy(message + sizeof(nonce), &timestamp, sizeof(timestamp));
    printf("Message despues de memcpy vale: ");
	for(int i=15; i>=0; i--) {
        printf("%02x ", message[i]);
    }
    printf("\n");

    // hmac_sha1(message, sizeof(message), key);
    // hmac_sha1(message, size_unsigned(message), key);
    hmac_sha1(nonce, timestamp, key);

    // if (strcmp((char*)r, (char*) sha1_value) != 0) {
    //     status = "FAILURE";
    //     printf("hey");
    // }
    // else {
    //     printf("hoy");
    //     status = "SUCCESS";
    // }
}

int main(int argc, char* argv[])
{
    // get_accounts(argv[1]);

    int sockfd, connfd ;  /* listening socket and connection socket file descriptors */
    unsigned int len;     /* length of client address */
    struct sockaddr_in servaddr, client; 
    
    int  len_rx = 0;                     /* received and sent length, in bytes */
    int total_bytes_rx = 0;
    int data_len = 0;
    unsigned char r[MAX_SHA1];
    char data[MAX_BUFF];             /*Total data readed*/
    char buff_tx[MAX_BUFF] = "Hello client, I am the server";
    char buff_rx[MAX_BUFF];   /* buffers for reception  */
    char login[MAX_LOGIN];
    uint64_t nonce;     /*random number generated cryptologically secure*/
    uint64_t timestamp;
    // time_t timestamp;

	if (argc != 2) {
		errx(EXIT_FAILURE, "Introduce 1 argument");
	}

    /* socket creation */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) 
        err(errno, "[SERVER-error]: socket creation failed.\n");
    else
        printf("[SERVER]: Socket successfully created..\n"); 
    
    /* clear structure */
    memset(&servaddr, 0, sizeof(servaddr));
  
    /* assign IP, SERV_PORT, IPV4 */
    servaddr.sin_family      = AF_INET; 
    servaddr.sin_addr.s_addr = 0; 
    servaddr.sin_port        = htons(PORT); 
    
    
    /* Bind socket */
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) 
        err(errno, "[SERVER-error]: socket bind failed.\n");
    else
        printf("[SERVER]: Socket successfully binded \n");
  
    /* Listen */
    if ((listen(sockfd, 100)) != 0) 
        err(errno, "[SERVER-error]: socket listen failed.\n");
    else
        printf("[SERVER]: Listening on SERV_PORT %d \n\n", ntohs(servaddr.sin_port) ); 
    
    len = sizeof(client); 
  
      /* Accept the data from incoming sockets in a iterative way */
    while(1)
    {
        connfd = accept(sockfd, (struct sockaddr *)&client, &len); 
        if (connfd < 0) 
            err(errno, "[SERVER-error]: connection not accepted.\n");
        else
        {   
            /*send a nonce to client*/
            nonce = csprng();
            write(connfd, &nonce, sizeof(nonce));
            
            while(1) /* read data from a client socket till it is closed */ 
            {  
                /* read client message, copy it into buffer */
                memset(data, 0, sizeof(data));
                do {
                    memset(buff_rx, 0, sizeof(buff_rx));
                    len_rx = read(connfd, buff_rx, sizeof(buff_rx));
                    total_bytes_rx += len_rx;
                    if(len_rx == -1)
                    {
                        memset(buff_rx, 0 , MAX_BUFF);
                        close(connfd);
                        errx(EXIT_FAILURE, "[SERVER-error]: connfd cannot be read.\n");
                    }
                    else if(len_rx == 0) /* if length is 0 client socket closed, then exit */
                    {
                        memset(buff_rx, 0 , MAX_BUFF);
                        close(connfd);
                        printf("[SERVER]: client socket closed \n\n");
                        break; 
                    }
                    memcpy(data + data_len, buff_rx, len_rx);
                    data_len += len_rx;
                } while (total_bytes_rx < sizeof(buff_rx));
                
                if(len_rx == 0) /* if length is 0 client socket closed, then exit */
                    break; 
                else
                {
                    // memset(r, 0, MAX_SHA1);
                    // memset(&timestamp, 0, sizeof(timestamp));
                    // memset(login, 0, MAX_LOGIN);
                    printf("Se leyeron: %d\n", total_bytes_rx);
                    memcpy(r, data, MAX_SHA1);
                    printf("Hmac: ");
                    print_digest(r);
                    memcpy(&timestamp, data + MAX_SHA1, sizeof(timestamp));
                    
                    memcpy(login, data + MAX_SHA1 + sizeof(timestamp), MAX_LOGIN);
                    printf("Login: %s \n", login);

                    check_hmacsha1(r, login, argv[1], timestamp, nonce);
                    write(connfd, buff_tx, strlen(buff_tx));
                    // printf("[SERVER]: %s \n", data);<
                }            
            }  
        }                      
    }    

} //COMPROBAR QUE ARGV ESTA BIEN. AL LEER SI DA ERROR ACABAR EL PROGRAMA???