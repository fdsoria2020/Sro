#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main() {
    int fd;
    char buffer[BUFFER_SIZE];
    char *data;
    ssize_t bytes_read;
    size_t total_bytes_read = 1;
    
    data = malloc(BUFFER_SIZE);
    memset(buffer, 0, BUFFER_SIZE);
    memset(data, 0, BUFFER_SIZE);
    fd = open("image.jpeg", O_RDONLY);

    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        total_bytes_read += bytes_read;
        if (total_bytes_read > BUFFER_SIZE){
            data = realloc(data, total_bytes_read);
            if (data == NULL) {
                perror("Error al asignar memoria");
                exit(EXIT_FAILURE);
            }
        }
        // memset(data, 0 ,total_bytes_read);
        printf("Esto es el buffer %s\nBytes read: %ld\nSizeof: %ld\nStrlen: %ld\n", buffer, bytes_read, sizeof(buffer), strlen(buffer));
        // printf("Esto es el data ANTES %s\n", data);
        strncat(data, buffer, bytes_read);
        // printf("Esto es el data DESPUES %s\n", data);
        memset(buffer, 0, BUFFER_SIZE);
    }

    if (bytes_read == -1) {
        perror("Error al leer el archivo");
        exit(EXIT_FAILURE);
    }

    printf("Se leyeron %zu bytes en total.\n", total_bytes_read);
    unsigned char* data_as_unsigned_char = (unsigned char*) data;
    // printf("Esto es el data FINAL %s\n", data_as_unsigned_char);
    // aquí podrías procesar o hacer algo con la variable 'data'

    free(data);
    close(fd);
    return 0;
}