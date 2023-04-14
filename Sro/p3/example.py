#include <stdio.h>
#include <string.h>
#include <err.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

enum {
    // MAX_BUFF = 12228,
    MAX_BUFF = 1024,
    MAX_KEY = 65,

};


size_t
get_elements(FILE *fd, size_t size){
    size_t count;
    long file_size;

    fseek(fd, 0, SEEK_END); // mover el puntero al final del archivo
    file_size = ftell(fd); // obtener la posición del puntero (el tamaño del archivo)
    rewind(fd); // mover el puntero al principio del archivo
    count = file_size / size; // calcular el número de elementos que se leerán
    
    return count;
}

int
size_unsigned(unsigned char str[]){
    int bytes = 0;
    for(int i = 0; str[i] != '\0'; i++){
        bytes+=sizeof(str[i]);
    }
    // while(str[bytes] != '\0') {
    //     bytes++;
    // }
    printf("En %s hay bytes: %d\n", str, bytes);
    return bytes;
}


unsigned char*
get_key(char*file){

    int fd;
    char buffer[MAX_KEY];
    char *data;
    ssize_t bytes_read;
    size_t total_bytes_read = 1;
    
    data = malloc(MAX_KEY);
    memset(buffer, 0, MAX_KEY);
    memset(data, 0, MAX_KEY);
    fd = open(file, O_RDONLY);

    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = read(fd, buffer, MAX_KEY)) > 0) {
        total_bytes_read += bytes_read;
        if (total_bytes_read > MAX_KEY){
            data = realloc(data, total_bytes_read);
            if (data == NULL) {
                perror("Error al asignar memoria");
                exit(EXIT_FAILURE);
            }
        }
        // printf("Esto es el buffer %s\n", buffer);
        // printf("Esto es el data ANTES %s\n", data);
        strncat(data, buffer, bytes_read);
        // printf("Esto es el data DESPUES %s\n", data);
        memset(buffer, 0, MAX_KEY);
    }

    if (bytes_read == -1) {
        perror("Error al leer el archivo");
        exit(EXIT_FAILURE);
    }

    printf("Se leyeron %zu bytes en total.\n", total_bytes_read);
    unsigned char* data_as_unsigned_char = (unsigned char*) data;
    printf("Esto es el data FINAL %s\n", data_as_unsigned_char);
    // aquí podrías procesar o hacer algo con la variable 'data'

    // free(data);
    close(fd);
    return data_as_unsigned_char;
}

void
create_sha1(char text[]){

    EVP_MD_CTX *mdctx;
    unsigned char sha1_value[EVP_MAX_MD_SIZE];
    unsigned int sz, i;

    // printf("Este es el fichero: %s\n", text);


    mdctx = EVP_MD_CTX_new();
    if (!EVP_DigestInit(mdctx, EVP_sha1())) {
        EVP_MD_CTX_free(mdctx);
        errx(1, "SHA1 EVP_DigestInit failed");
    }
    if (!EVP_DigestUpdate(mdctx, text, strlen(text))) {
        printf("Message digest update failed.\n");
        EVP_MD_CTX_free(mdctx);
        exit(1);
    }

    // if (!EVP_DigestUpdate(mdctx, text, strlen(text))) {
    //     printf("Message digest update failed.\n");
    //     EVP_MD_CTX_free(mdctx);
    //     exit(1);
    // }

    sz = EVP_MD_size(EVP_sha1());
    if (!EVP_DigestFinal_ex(mdctx, sha1_value, &sz) || sz !=EVP_MD_size(EVP_sha1())) {
        printf("Message digest finalization failed.\n");
        EVP_MD_CTX_free(mdctx);
        exit(1);
    }
    EVP_MD_CTX_free(mdctx);

    printf("Digest is: ");
    for (i = 0; i < sz; i++)
        printf("%02x", sha1_value[i]);
    printf("\n");


}

EVP_MD_CTX*
init_sha1(){

    EVP_MD_CTX *mdctx;

    mdctx = EVP_MD_CTX_new();
    if (!EVP_DigestInit(mdctx, EVP_sha1())) {
        EVP_MD_CTX_free(mdctx);
        errx(1, "SHA1 EVP_DigestInit failed");
    }
    return mdctx;
}

void
update_sha1(EVP_MD_CTX *mdctx, unsigned char text[], int size_text){

    if (!EVP_DigestUpdate(mdctx, text, size_text)) {
        printf("Message digest update failed.\n");
        EVP_MD_CTX_free(mdctx);
        exit(1);
    }
}

void
finish_sha1(EVP_MD_CTX *mdctx, unsigned char sha1_value[]){

    // unsigned char sha1_value[EVP_MAX_MD_SIZE];
    unsigned int sz, i;

    sz = EVP_MD_size(EVP_sha1());
    if (!EVP_DigestFinal_ex(mdctx, sha1_value, &sz) || sz !=EVP_MD_size(EVP_sha1())) {
        printf("Message digest finalization failed.\n");
        EVP_MD_CTX_free(mdctx);
        exit(1);
    }
    EVP_MD_CTX_free(mdctx);

    printf("Digest is: ");
    for (i = 0; i < sz; i++)
        printf("%02x", sha1_value[i]);
    printf("\n");
}

int
reset_key(unsigned char key[], unsigned char tk[]){

    EVP_MD_CTX *tctx;
    memset(tk, 0 , 64);
    int i;
    printf("La key vale: %s\n", key);
    tctx = init_sha1();
    update_sha1(tctx, key, size_unsigned(key));
    finish_sha1(tctx, tk);
    key = tk;
    // printf("DESPUES de RESET key: %s\n", key);
    i = size_unsigned(key);
    // key_len = 16;
    return i;
}

void
hmac_sha1(char* text, unsigned char key[]){

    unsigned char k_ipad[65];    // key XORd with ipad
    unsigned char k_opad[65];    // outer padding - key XORd with opad
    unsigned char tk[64];
	unsigned char buffer[MAX_BUFF];
    unsigned char sha1_value[EVP_MAX_MD_SIZE];
    memset(sha1_value, 0, 64);

    size_t size_readed = 1;
	size_t size = sizeof(unsigned char);
    size_t elements;
	int fd;
    EVP_MD_CTX *mdctx;
    int i;
    int key_reseted = 0;
    int key_len = size_unsigned(key);

    if (key_len <= 20)
        warnx("warning: key is too short (should be longer than 20 bytes)");
    else if (key_len > 64){
        printf("ANTES de RESET key: %s\n", key);
        key_len = reset_key(key, tk);
        free(key);
        key = tk;
        key_reseted = 1;
        printf("DESPUES de RESET key: %s\n", key);
    }
    bzero( k_ipad, sizeof k_ipad);
    bzero( k_opad, sizeof k_opad);
    bcopy( key, k_ipad, key_len);
    bcopy( key, k_opad, key_len);

    for (i=0; i<64; i++) {
            k_ipad[i] ^= 0x36;
            k_opad[i] ^= 0x5c;
    }

	
    fd = open(text, O_RDONLY);
    if (fd == -1){
        err(errno, "No se pudo abrir el fichero");
    }
    else{
        // elements = MAX_BUFF;

        memset(buffer, 0, MAX_BUFF);    
        mdctx = init_sha1();
        update_sha1(mdctx, k_ipad, size_unsigned(k_ipad));
        while ((size_readed = read(fd, buffer, MAX_BUFF)) > 0) {
            update_sha1(mdctx, buffer, size_readed);
            printf("Size readed: %zu\n y el bufffer : %s\n y su longuitud es: %d\n\n", size_readed, buffer, size_unsigned(buffer));  // prints as unsigned decimal
	        memset(buffer, 0, MAX_BUFF);
        }
        finish_sha1(mdctx, sha1_value);
        close(fd);
        mdctx = init_sha1();
        update_sha1(mdctx, k_opad, size_unsigned(k_opad));
        update_sha1(mdctx, sha1_value, size_unsigned(sha1_value));
        finish_sha1(mdctx, sha1_value);
        if (!key_reseted)
            free(key);
    }
}

int main(int argc, char *argv[])
{
    hmac_sha1(argv[1], get_key(argv[2]));
    // reset_key(get_key(argv[2]));
	// get_key(argv[2]);
    // create_sha1(argv[1]);   
}