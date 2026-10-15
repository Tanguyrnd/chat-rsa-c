#ifndef RSA_H
#define RSA_H

#include <stdbool.h>
#include <stdint.h>

#define RSA_PRIME_MIN 32768
#define RSA_PRIME_MAX 65536
#define RSA_PUBLIC_EXPONENT 65537
#define RSA_MAX_BLOCK_VALUE 65535

struct PublicKey {
    uint64_t exponent;
    uint64_t modulus;
};

struct PrivateKey {
    uint64_t exponent;
    uint64_t modulus;
};

void rsa_seed_random(void);
void rsa_generate_keys(struct PublicKey* public_key, struct PrivateKey* private_key);
bool rsa_is_valid_public_key(struct PublicKey key);
uint64_t rsa_encrypt_block(struct PublicKey key, uint64_t block);
uint64_t rsa_decrypt_block(struct PrivateKey key, uint64_t block);

#endif