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
#include <sys/time.h> // Para utilizar struct timeval
#include <fcntl.h> // Para utilizar la función fcntl()


/* strings / errors*/
#include <string.h>
#include <err.h>

/*openssl*/
#include <openssl/rand.h>

#include "tools.h"


enum{
    PORT = 9999,
    MAX_BUFF = 284,
    MAX_LOGIN = 256,
    MAX_CLIENTS = 100,
    MAX_SHA1 = 20,
};

#define SERVER_ADDRESS "127.0.0.1"    /* server IP */

/*
    Create a nonce using a cryptographically secure pseudo random number generator 
*/
uint64_t 
csprng()
{
    int numbers[MAX_CLIENTS]; // números generados
    time_t timestamps[MAX_CLIENTS]; // tiempos de generación
    int num_numbers = 0; // número actual de números generados y almacenados
    uint64_t number;

    while (1)  {
        if (RAND_bytes((unsigned char*)&number, sizeof(number)) != 1) {
            // Error al generar números aleatorios
            errx(EXIT_FAILURE, "Error creating a nonce");
        }
        if (num_numbers == 0)  {
            numbers[num_numbers] = number;
            timestamps[num_numbers] = time(NULL);
            return number;
        }
        // comprobar si el número ya ha sido generado en los últimos 10 minutos
        for (int i = 0; i < num_numbers; i++) {
            // comprobar si el número fue generado en los últimos 10 minutos 
            if (numbers[i] == number && time(NULL) - timestamps[i] < 600)  {
                break;
            }
            // actualizar el tiempo de generación si el número fue generado hace más de 10 minutos
            else if (numbers[i] == number && time(NULL) - timestamps[i] > 600)  { 
                timestamps[i] = time(NULL);
                return number;
            }
        }
    }
}

int is_hexa(char *s) {
  char *end;
  strtol(s, &end, 16);
  return (*s != '\0' && *end == '\0');
}

void
check_account(char* account)  {
    char* aux;
    char buff[MAX_BUFF];
    char* token;

    memcpy(buff, account, MAX_BUFF);
    if (!strchr(buff, ':'))
        errx(EXIT_FAILURE, "[ERROR]: Format in file of accounts doesnt include ':'");
    token = strtok_r(buff, ":", &aux);
    if (token == NULL)
        errx(EXIT_FAILURE, "[ERROR]: strtok_r failed");
    else if (strlen(token) > 256)
        errx(EXIT_FAILURE, "[ERROR]: User is more than 256 bytes in file of accounts");
    token = strtok_r(NULL, ":", &aux);
    if (token == NULL)
        errx(EXIT_FAILURE, "[ERROR]: strtok_r failed");
    else if (!is_hexa(token) || strlen(token) != 40)
        errx(EXIT_FAILURE, "[ERROR]: Key isnt 20 bytes of hexa in file of accounts");
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

char*
get_key(char* account)
{
    char* token;
    char* aux;
    token = strtok_r(account, ":", &aux);
    token = strtok_r(NULL, ":", &aux);
    // return (unsigned char*)token;
    return token;
}

char *
get_accounts(char* file, char* user, char * buffer)
{
    FILE *fd;
    int found = 0;
    fd = fopen(file, "rb");
    if (fd == NULL)
        err(EXIT_FAILURE, "Cannot open the file");

    while (fgets(buffer, MAX_BUFF, fd) != NULL || !found) {
        if (ferror(fd))  {
            fclose(fd);
            err(EXIT_FAILURE, "Error reading the file");
        }
        buffer[strcspn(buffer, "\r\n")] = '\0';
        check_account(buffer);
        if (strstr(buffer, user) != NULL) {
            found = 1;
        }
    }

    if (fclose(fd) != 0)
        err(EXIT_FAILURE, "Error closing the file");
    
    return buffer;
}

void
check_hmacsha1(unsigned char* r, char *user, char *file, uint64_t timestamp, uint64_t nonce, char *status)
{
    unsigned char sha1_value[MAX_SHA1];
    char* account;
    unsigned char key[] = "3f786850e387550fdab836ed7e6dc881de230b1b";
    unsigned char key_hex[20];
    // char * key;
    char * endptr;

    char buffer[MAX_BUFF];
    unsigned char message[2*sizeof(uint64_t)];
    // unsigned char message[50] = "estos son los datos";

    printf("El nonce vale: %02lX\n", nonce);
    printf("Timestamp: %02lX \n", timestamp);
    account = get_accounts(file, user, buffer);
    // printf("EL tamaño de account es: %ld\n", sizeof(account));

    // key = get_key(account);
    printf("La clave es: %s - %d bytes\n", key, size_unsigned(key));
    // int key_hex = (int)strtol(key, NULL, 16);
    char hex[3];
    int j = 0;
    memset(key_hex, 0, sizeof(key_hex));
    for (int i = 0; i < 40; i+=2) {
        memset(hex, 0, sizeof(hex));
        hex[0] = key[i];
        hex[1] = key[i+1];
        key_hex[j] = strtol(hex, &endptr, 16);
        if (*endptr != '\0') {
            // Manejar errores de conversión
            printf("Error en la conversión del byte %d\n", i);
            break;
        }
        j++;
    }
    printf("Key en hexa: ");
    for (int i = 0; i < 20; i++) {
        printf("%02x", key_hex[i]);
    }
    printf("\n");
    memcpy(message, &nonce, sizeof(nonce));
	memcpy(message + sizeof(nonce), &timestamp, sizeof(timestamp));
    printf("Message despues de memcpy vale: ");
	for(int i=15; i>=0; i--) {
        printf("%02x ", message[i]);
    }
    printf("\n");
    hmac_sha1(message, 16, key_hex, sha1_value);
    // hmac_sha1(message, sizeof(message), key);
    // hmac_sha1(nonce, timestamp, key, sha1_value);

}

int main(int argc, char* argv[])
{
    int sockfd, connfd ;  /* listening socket and connection socket file descriptors */
    unsigned int len;     /* length of client address */
    struct sockaddr_in servaddr, client; 
    
    int  len_rx = 0;                     /* received and sent length, in bytes */
    int data_len = 0;
    unsigned char r[MAX_SHA1];
    char data[MAX_BUFF];             /*Total data readed*/
    char buff_tx[MAX_BUFF];
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
    if ((listen(sockfd, MAX_CLIENTS)) != 0) 
        err(errno, "[SERVER-error]: socket listen failed.\n");
    else {
        printf("[SERVER]: Listening on SERV_PORT %d \n\n", ntohs(servaddr.sin_port)); 
    }
    len = sizeof(client); 
  
      /* Accept the data from incoming sockets in a iterative way */
    while(1)
    {
        connfd = accept(sockfd, (struct sockaddr *)&client, &len); 
        if (connfd < 0) 
            err(errno, "[SERVER-error]: connection not accepted.\n");
        printf("[SERVER]: Connection accepted\n"); 

        /*Configure a timeout*/
        struct timeval timeout;
        timeout.tv_sec = 30;  // 30 segundos
        timeout.tv_usec = 0;

        if (setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            errx(EXIT_FAILURE, "[SERVER-error]: setsockopt() failed.\n");
        }

        /*send a nonce to client*/
        nonce = csprng();
        write(connfd, &nonce, sizeof(nonce));
        
        /* read data from a client socket and copy it into buffer till it is closed */ 
        int total_bytes_rx = 0;
        memset(data, 0, sizeof(data));
        do {
            memset(buff_rx, 0, sizeof(buff_rx));
            len_rx = read(connfd, buff_rx, sizeof(buff_rx));
            if (len_rx == -1 && errno == EAGAIN) { /*If client doesnt write in 30 secs close socket*/
                warnx("[SERVER-error]: Timeout in lecture, closing connection.\n");
                close(connfd);
                break;
            }
            else if(len_rx == -1)  {
                memset(buff_rx, 0 , MAX_BUFF);
                close(connfd);
                errx(EXIT_FAILURE, "[SERVER-error]: connfd cannot be read.\n");
            }
            else if(len_rx == 0)  { /* if length is 0 client socket closed, then exit */
                memset(buff_rx, 0 , MAX_BUFF);
                close(connfd);
                printf("[SERVER]: client socket closed \n\n");
                break; 
            }
            memcpy(data + total_bytes_rx, buff_rx, len_rx);
            total_bytes_rx += len_rx;
        } while (total_bytes_rx < sizeof(buff_rx));
        printf("La len leida es %d\n\n", len_rx);
        if(len_rx > 0)  {/* if length is <=0 client socket closed, then exit */
            printf("Se leyeron: %d\n", total_bytes_rx);
            memcpy(r, data, MAX_SHA1);
            printf("Hmac: ");
            print_digest(r);
            memcpy(&timestamp, data + MAX_SHA1, sizeof(timestamp));
            
            memcpy(login, data + MAX_SHA1 + sizeof(timestamp), MAX_LOGIN);
            printf("Login: %s \n", login);

            check_hmacsha1(r, login, argv[1], timestamp, nonce, buff_tx);
            write(connfd, buff_tx, strlen(buff_tx));
            // printf("[SERVER]: %s \n", data);<
        }            
    }    
} //COMPROBAR QUE ARGV ESTA BIEN. AL LEER SI DA ERROR ACABAR EL PROGRAMA???