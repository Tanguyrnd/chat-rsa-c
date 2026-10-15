#ifndef RSA_TEXT_H
#define RSA_TEXT_H

#include <stddef.h>

#include "rsa.h"

#define RSA_MAX_PLAIN_LENGTH 128

int rsa_encrypt_text(struct PublicKey key, const char* plain, char* cipher, size_t cipher_size);
int rsa_decrypt_text(struct PrivateKey key, const char* cipher, char* plain, size_t plain_size);

#endif