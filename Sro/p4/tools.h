#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>

void
print_digest(unsigned char sha1_value[]);

int
size_unsigned(unsigned char str[]);

EVP_MD_CTX *
init_sha1();

void
update_sha1(EVP_MD_CTX * mdctx, unsigned char text[], int size_text);

void
finish_sha1(EVP_MD_CTX * mdctx, unsigned char sha1_value[]);

void
reset_key(unsigned char key[], int key_size, unsigned char tk[]);

void
second_hash(unsigned char k_opad[], unsigned char sha1_value[]);

void
first_hash(unsigned char k_ipad[], unsigned char sha1_value[], unsigned char *text, int text_len);

void
key_xor(unsigned char k_pad[], unsigned char key[], int key_len, int const_);

void
hmac_sha1(unsigned char *text, int text_len, unsigned char *key, unsigned char *sha1_value);
