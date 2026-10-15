#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "number_theory.h"
#include "rsa.h"

void rsa_seed_random(void) {
    // Le OU exclusif (^) avec le Process ID garantit des clés différentes même si plusieurs instances de l'application sont lancées exactement dans la même seconde
    unsigned int graine = (unsigned int)time(NULL) ^ (unsigned int)getpid();
    srand(graine);
}

void rsa_generate_keys(struct PublicKey* public_key, struct PrivateKey* private_key) {
    while (1) {
        uint64_t p = random_prime(RSA_PRIME_MIN, RSA_PRIME_MAX);
        uint64_t q = random_prime(RSA_PRIME_MIN, RSA_PRIME_MAX);
        
        if (p == q) continue;
        
        uint64_t phi = (p - 1) * (q - 1);
        // Sécurité : s'assure que l'exposant public est premier avec phi, condition absolue pour que l'inversion modulaire (la clé privée) puisse exister
        if (gcd(RSA_PUBLIC_EXPONENT, phi) != 1) continue;
        
        uint64_t modulus = p * q;
        
        public_key->exponent = RSA_PUBLIC_EXPONENT;
        public_key->modulus = modulus;
        
        private_key->exponent = mod_inverse(RSA_PUBLIC_EXPONENT, phi);
        private_key->modulus = modulus;
        return;
    }
}

bool rsa_is_valid_public_key(struct PublicKey key) {
    if (!key.exponent) return false;
    if (key.modulus <= RSA_MAX_BLOCK_VALUE) return false;
    if (key.modulus > UINT32_MAX) return false;
    
    return true;
}

uint64_t rsa_encrypt_block(struct PublicKey key, uint64_t block) {
    return mod_pow(block, key.exponent, key.modulus);
}

uint64_t rsa_decrypt_block(struct PrivateKey key, uint64_t block) {
    return mod_pow(block, key.exponent, key.modulus);
}