#include <stdio.h>
#include <string.h>
#include <err.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

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
size_unsigned(unsigned char str[])
{
	int bytes = 0;

	while (str[bytes] != '\0') {
		bytes++;
	}
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

unsigned char *
get_key(char *file)
{
	int fd;
	char buffer[MAX_KEY];
	char *data;
	ssize_t bytes_read;
	size_t total_bytes_read = 1;

	data = malloc(MAX_KEY);
	if (data == NULL)
		errx(EXIT_FAILURE, "Cannot allocate memory");
	memset(data, 0, MAX_KEY);

	fd = open(file, O_RDONLY);
	if (fd == -1) {
		perror("Error opening the file");
		exit(EXIT_FAILURE);
	}

	memset(buffer, 0, MAX_KEY);
	while ((bytes_read = read(fd, buffer, MAX_KEY)) > 0) {
		total_bytes_read += bytes_read;
		if (total_bytes_read >= MAX_KEY) {
			data = realloc(data, total_bytes_read);
			if (data == NULL) {
				close(fd);
				free(data);
				errx(EXIT_FAILURE, "Cannot allocate memory");
			}
		}
		strncat(data, buffer, bytes_read);
		memset(buffer, 0, MAX_KEY);
	}

	if (bytes_read == -1) {
		close(fd);
		free(data);
		errx(EXIT_FAILURE, "Error reading the file");
	}
	close(fd);
	unsigned char *data_as_unsigned_char = (unsigned char *)data;

	return data_as_unsigned_char;
}

void
second_hash(unsigned char k_opad[], unsigned char sha1_value[])
{
	EVP_MD_CTX *mdctx;

	mdctx = init_sha1();
	update_sha1(mdctx, k_opad, size_unsigned(k_opad));
	update_sha1(mdctx, sha1_value, MAX_SHA1);
	memset(sha1_value, 0, MAX_SHA1);
	finish_sha1(mdctx, sha1_value);
	print_digest(sha1_value);
}

void
first_hash(unsigned char k_ipad[], unsigned char sha1_value[], FILE * fd)
{
	unsigned char buffer[MAX_BUFF];
	size_t size = sizeof(unsigned char);
	size_t elements = MAX_BUFF;
	size_t size_readed = 1;
	EVP_MD_CTX *mdctx;

	memset(buffer, 0, MAX_BUFF);
	mdctx = init_sha1();
	update_sha1(mdctx, k_ipad, size_unsigned(k_ipad));
	while (!feof(fd)) {
		size_readed = fread(buffer, size, elements, fd);
		if (ferror(fd)) {
			finish_sha1(mdctx, sha1_value);
			errx(1, "Error reading the file");
		}
		update_sha1(mdctx, buffer, size_readed);
		memset(buffer, 0, MAX_BUFF);
	}
	finish_sha1(mdctx, sha1_value);
	fclose(fd);

}

void
key_xor(unsigned char k_pad[], unsigned char key[], int key_len, int const_)
{
	int i;

	bzero(k_pad, sizeof(char) * (MAX_KEY));
	bcopy(key, k_pad, key_len);
	for (i = 0; i < 64; i++) {
		k_pad[i] ^= const_;
	}
}

void
hmac_sha1(char *text, unsigned char *key)
{
	unsigned char k_ipad[MAX_KEY];	// key XORd with ipad
	unsigned char k_opad[MAX_KEY];	// outer padding - key XORd with opad
	unsigned char sha1_value[MAX_SHA1];
	unsigned char tk[MAX_SHA1];
	FILE *fd;
	int key_len = size_unsigned(key);

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

	fd = fopen(text, "rb");
	if (fd == NULL) {
		errx(errno, "Cannot open the file");
	} else {
		first_hash(k_ipad, sha1_value, fd);
		second_hash(k_opad, sha1_value);
	}
}

int
main(int argc, char *argv[])
{
	if (argc != 3) {
		errx(EXIT_FAILURE, "Too many arguments, introduce 2");
	}
	hmac_sha1(argv[1], get_key(argv[2]));
}
