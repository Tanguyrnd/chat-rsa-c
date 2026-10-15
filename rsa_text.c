#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rsa.h"
#include "rsa_text.h"

int rsa_encrypt_text(struct PublicKey key, const char* plain, char* cipher, size_t cipher_size) {
    size_t taille_clair = strlen(plain);
    size_t taille_chiffre = 0;
    cipher[0] = '\0';
    
    // On regroupe les octets par 2 pour limiter la taille de la trame finale envoyée sur le réseau
    for (size_t i = 0; i < taille_clair; i += 2) {
        unsigned char octet_fort = (unsigned char)plain[i];
        unsigned char octet_faible = (i + 1 < taille_clair) ? (unsigned char)plain[i + 1] : 0;
        
        uint64_t block = octet_fort * 256 + octet_faible;
        uint64_t block_chiffre = rsa_encrypt_block(key, block);
        
        size_t place = cipher_size - taille_chiffre;
        int ecrit = snprintf(cipher + taille_chiffre, place, "%" PRIu64 " ", block_chiffre);
        
        if (ecrit < 0 || (size_t)ecrit >= place) return -1;
        
        taille_chiffre += (size_t)ecrit;
    }
    
    return 0;
}

int rsa_decrypt_text(struct PrivateKey key, const char* cipher, char* plain, size_t plain_size) {
    size_t taille_clair = 0;
    const char* ptr = cipher;
    
    while (1) {
        while (*ptr == ' ') ptr++;
        if (!*ptr) break;
        
        char* fin = NULL;
        uint64_t block_chiffre = strtoull(ptr, &fin, 10);
        if (fin == ptr) return -1;
        
        ptr = fin;
        uint64_t block = rsa_decrypt_block(key, block_chiffre);
        
        // empêche un client malveillant de saturer la mémoire avec des blocs géants
        if (block > RSA_MAX_BLOCK_VALUE) return -1;
        if (taille_clair + 2 >= plain_size) return -1;
        
        plain[taille_clair] = (char)(block / 256);
        plain[taille_clair + 1] = (char)(block % 256);
        taille_clair += 2;
    }
    
    plain[taille_clair] = '\0';
    return 0;
}