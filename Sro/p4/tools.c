#include <stdio.h>
#include <string.h>
#include <err.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "tools.h"
#include <stdint.h>


enum {
	MAX_BUFF = 1024,
	MAX_KEY = 65,
	MAX_SHA1 = 20,
	IPAD = 0X36,
	OPAD = 0x5c,
};

void
print_digest(unsigned char sha1_value[])
{
	unsigned int sz, i;

	sz = EVP_MD_size(EVP_sha1());
	for (i = 0; i < sz; i++)
		printf("%02x", sha1_value[i]);
	printf("\n");
}

int
size_unsigned(unsigned char *str)
{
	int bytes = 0;

	while (str[bytes] != '\0') {
		bytes++;
	}
	printf("El string '%s' tiene %d bytes\n", str, bytes);
	return bytes;
}

EVP_MD_CTX *
init_sha1()
{
	EVP_MD_CTX *mdctx;

	mdctx = EVP_MD_CTX_new();
	if (!EVP_DigestInit(mdctx, EVP_sha1())) {
		EVP_MD_CTX_free(mdctx);
		errx(EXIT_FAILURE, "SHA1 EVP_DigestInit failed");
	}
	return mdctx;
}

void
update_sha1(EVP_MD_CTX * mdctx, unsigned char text[], int size_text)
{
	if (!EVP_DigestUpdate(mdctx, text, size_text)) {
		EVP_MD_CTX_free(mdctx);
		errx(EXIT_FAILURE, "Message digest update failed.\n");
	}
}

void
finish_sha1(EVP_MD_CTX * mdctx, unsigned char sha1_value[])
{
	unsigned int sz;

	sz = EVP_MD_size(EVP_sha1());
	if (!EVP_DigestFinal_ex(mdctx, sha1_value, &sz)
	    || sz != EVP_MD_size(EVP_sha1())) {
		EVP_MD_CTX_free(mdctx);
		errx(EXIT_FAILURE, "Message digest finalization failed.\n");
	}
	EVP_MD_CTX_free(mdctx);
}

void
reset_key(unsigned char key[], int key_size, unsigned char tk[])
{
	EVP_MD_CTX *tctx;

	tctx = init_sha1();
	update_sha1(tctx, key, key_size);
	finish_sha1(tctx, tk);
}

void
second_hash(unsigned char k_opad[], unsigned char sha1_value[])
{
	EVP_MD_CTX *mdctx;

	mdctx = init_sha1();
	update_sha1(mdctx, k_opad, 64); //k_opad has 64 bytes '\0'
	update_sha1(mdctx, sha1_value, MAX_SHA1);
	memset(sha1_value, 0, MAX_SHA1);
	finish_sha1(mdctx, sha1_value);
	print_digest(sha1_value);
}

void
first_hash(unsigned char k_ipad[], unsigned char sha1_value[], unsigned char *text, int text_len)
{
	EVP_MD_CTX *mdctx;

	mdctx = init_sha1();
	update_sha1(mdctx, k_ipad, 64); //k_ipad has 64 bytes '\0'
	if (!EVP_DigestUpdate(mdctx, text, text_len)) {
		EVP_MD_CTX_free(mdctx);
		errx(EXIT_FAILURE, "Message digest update failed.\n");
	}
	finish_sha1(mdctx, sha1_value);

}

void
key_xor(unsigned char k_pad[], unsigned char key[], int key_len, int const_)
{
	int i;

	bzero(k_pad, sizeof(char) * (MAX_KEY));
	bcopy(key, k_pad, key_len);
	// printf("El pad '%s' antes \n", k_pad);
	// for (i = 0; i < 64; i++) {
	// 	k_pad[i] ^= const_;
    //     printf("%02X ", k_pad[i]);
	// }
	// printf("El pad '%s' despues\n", k_pad);


}

void
hmac_sha1(unsigned char *text, int text_len, unsigned char* key, unsigned char *sha1_value)
{
	unsigned char k_ipad[MAX_KEY];	// key XORd with ipad
	unsigned char k_opad[MAX_KEY];	// outer padding - key XORd with opad
	// unsigned char sha1_value[MAX_SHA1];
	unsigned char tk[MAX_SHA1];
	int key_len = size_unsigned(key);
	key_len = 20;
	memset(sha1_value, 0, MAX_SHA1);
	memset(tk, 0, MAX_SHA1);

	if (key_len <= 20)
		warnx
		    ("warning: key is too short (should be longer than 20 bytes)");
	else if (key_len > 64) {
		reset_key(key, key_len, tk);
		free(key);
		key = tk;
		key_len = MAX_SHA1;
	}
	key_xor(k_ipad, key, key_len, IPAD);
	key_xor(k_opad, key, key_len, OPAD);

    first_hash(k_ipad, sha1_value, text, text_len);
    second_hash(k_opad, sha1_value);
}

// int
// main(int argc, char *argv[])
// {
// 	uint64_t nonce = 7836713044604439961;
// 	uint64_t time = 1680715946;
// 	unsigned char hmac[40] = "782f07b0d7f3bb8a978f9b83cf285c1c0481ab20";
// 	// if (argc != 3) {
// 	// 	errx(EXIT_FAILURE, "Too many arguments, introduce 2");
// 	// }
// 	// unsigned char texto [1024];
// 	unsigned char texto1 [1024];
// 	unsigned char texto2 [1024];
// 	printf("El nonce vale: %02lX\n", nonce);
// 	printf("Timestamp: %02lX \n", time);

// 	memcpy(texto1, &nonce, sizeof(nonce));
// 	memcpy(texto2, &time, sizeof(time));
// 	// memcpy(texto + sizeof(nonce), &time, sizeof(time));
	
// 	printf("El texto vale:");
// 	for(int i=15; i>=0; i--) {
//         printf("%02x ", texto1[i]);
//     }
// 	printf("\n");
// 	printf("El texto vale:");
// 	for(int i=15; i>=0; i--) {
//         printf("%02x ", texto2[i]);
//     }
// 	printf("\n");

// 	unsigned char clave [1024] = "3f786850e387550fdab836ed7e6dc881de23001b";
// 	// for (int i = 0; i < 64; i++) {
// 	// // clave[i] ^= IPAD;
// 	// printf("%02X", clave[i]);
// 	// }
// 	// hmac_sha1(argv[1], size_unsigned(argv[1]), argv[2]);
// 	hmac_sha1(nonce, time, clave);
// }
