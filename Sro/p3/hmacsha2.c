#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

enum{
    BUFFER = 4096,
    MAX_KEY = 100000000, //100 megabytes
    BLOCK_LENGHT = 64,
    SHA1_LENGHT = 20,
    CONST1 = 0x5c,
    CONST2 = 0X36,
    NARGS = 3
};

int
getBSize(unsigned char *str){
    int bytes;

    bytes = 0;
    for(; *str != '\0'; str++){
        bytes += sizeof(*str);
    }
    return bytes;
}


void
rd_Key(char *fich, unsigned char *key){
    int fd_r, b_rd;
    unsigned char aux[BUFFER];

    memset(aux, 0, sizeof(char)*BUFFER);
    strcpy((char*)key, (char*)aux);
    fd_r = open(fich, O_RDONLY);
    b_rd = 1;
    while(b_rd != 0){
        memset(aux, 0, sizeof(char)*BUFFER);
        b_rd = read(fd_r, (char*)aux, BUFFER);
        if(b_rd == -1){
            free(key);
            err(EXIT_FAILURE, "Cannot read");        
        }
        strcat((char*)key, (char*)aux);
    }
    close(fd_r);
}

int
updateByFich(char *fich, EVP_MD_CTX *context){
    int fd_r, b_rd;
    char readed[BUFFER];
    int error = 1;

    b_rd = 1;
    fd_r = open(fich, O_RDONLY);
    while ((b_rd != 0) && (error == 1)) {
        memset(readed, '\0', sizeof(char)*BUFFER);
        b_rd = read(fd_r, readed, BUFFER);
        if (b_rd == -1){
            error = 0;
        }else{
            if(!EVP_DigestUpdate(context, readed, b_rd)){
            error = 0;
            }
        }
    }
    close(fd_r);
    return error;
}

void
keyXORconst(unsigned char result[], unsigned char *key, int constx){
    int i;

    bzero(result, sizeof(char)*(BLOCK_LENGHT + 1));
    bcopy(key, result, getBSize(key));
    for(i = 0; i < BLOCK_LENGHT; i++){
        result[i] ^= constx;
    }
}

int
hashCalculator(unsigned char k_constx[], char *str, unsigned char result[], int control, EVP_MD_CTX *context){
    unsigned int sz;

    if(!EVP_DigestInit(context, EVP_sha1())){
        return 0;
    }
    if(k_constx){
        if(!EVP_DigestUpdate(context, k_constx, getBSize(k_constx))){
            return 0;
        }
    }
    if(control == 1){
        if(!updateByFich(str, context)){
            return 0;        
        }
    }//else if(control == 0){
    //     if(!EVP_DigestUpdate(context, str, SHA1_LENGHT)){
    //         return 0;
    //     }
    // }
    sz = EVP_MD_size(EVP_sha1());
    if(!EVP_DigestFinal_ex(context, result, &sz) || sz != EVP_MD_size(EVP_sha1())){
        return 0;
    }
    return 1;
}

void
printHMacSha1(unsigned char hmacsha1[]){
    int i;

    printf("Result: ");
    for(i = 0; i < SHA1_LENGHT; i++){
        printf("%02x", hmacsha1[i]);
    }
    printf("\n");
}

int
checkArgsOk(char *argv[], int argc){
    int i;
    struct stat fichdata;

    if (argc != 3){
        errx(EXIT_FAILURE, "Incorrect number of arguments, only 2 are allowed");    
    }
    for(i = 1; i < NARGS; i++){
        if(access(argv[i], R_OK)){
            err(EXIT_FAILURE, "Cannot open %s", argv[i]);
        }
    }
    if (stat(argv[2], &fichdata) == -1) {
        err(EXIT_FAILURE, "stat");
    }
    if(fichdata.st_size > MAX_KEY){
        errx(EXIT_FAILURE, "Key longer than 100MB, try to put one shorter");    
    }
    return fichdata.st_size;
}

unsigned char*
initKey(int keysize){
    unsigned char *key;

    key = (unsigned char *)malloc(sizeof(unsigned char)*(keysize + 1));
    if(key == NULL){
        err(EXIT_FAILURE, "Cannot allocate memory");    
    }
    bzero(key, sizeof(unsigned char)*(keysize + 1));
    return key;
}

int main(int argc, char *argv[]){
    unsigned char *key;   
    unsigned char k_ipad[BLOCK_LENGHT + 1]; // Key XOR CONST2. +1 for the '/0'
    unsigned char k_opad[BLOCK_LENGHT + 1]; // Key XOR CONST1. +1 for the '/0'
    unsigned char sha1[SHA1_LENGHT];        // Hash of k_ipad concated with Data
    unsigned char hmacsha1[SHA1_LENGHT];    // Hash of k_opad concated with Sha1
    EVP_MD_CTX *context;
    int keysize;

    keysize = checkArgsOk(argv, argc);
    key = initKey(keysize);
    context = EVP_MD_CTX_new();
    if(!context){
        free(key);
        err(EXIT_FAILURE, "Cannot allocate memory");    
    }
    if(keysize < SHA1_LENGHT){
        warnx("Warning: key is too short(should be longer than 20 bytes)");
    }
    if(keysize > BLOCK_LENGHT){
        printf("Key longer than 64 bytes: %d bytes\n", keysize);
        if(!hashCalculator(NULL, argv[2], key, 1, context)){
            free(key);
            EVP_MD_CTX_free(context);
            err(EXIT_FAILURE, "Error at hashing");
        }
    }else{
        rd_Key(argv[2], key);    
    }
    keyXORconst(k_opad, key, CONST1);
    keyXORconst(k_ipad, key, CONST2);
    free(key);
    if(!hashCalculator(k_ipad, argv[1], sha1, 1, context)){//H(k_ipad)||M saved in sha1
        EVP_MD_CTX_free(context);
        err(EXIT_FAILURE, "Error at hashing");
    }
    printf("El sha1: %d bytes\n", getBSize(sha1));
    printHMacSha1(sha1);

    if(!hashCalculator(k_opad, (char*)sha1, hmacsha1, 0, context)){//k_opad||sha1 saved in hmacsha1
        EVP_MD_CTX_free(context);
        err(EXIT_FAILURE, "Error at hashing");
    }
    EVP_MD_CTX_free(context);

    printHMacSha1(hmacsha1);
    exit(EXIT_SUCCESS);
}
