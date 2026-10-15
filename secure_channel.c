#include <stdio.h>

#include "common.h"
#include "rsa.h"
#include "rsa_text.h"
#include "secure_channel.h"

int channel_exchange_keys(int socket, struct PublicKey own_public_key, struct PublicKey* peer_public_key) {
    char ligne[MAX_MESSAGE_SIZE];
    
    // échange en format texte pour éviter les bugs d'endianness (little/big endian) entre différents OS
    snprintf(ligne, sizeof(ligne), "%llu %llu",
             (unsigned long long)own_public_key.exponent,
             (unsigned long long)own_public_key.modulus);
             
    if (send_message(socket, ligne) < 0) return -1;
    if (receive_message(socket, ligne, sizeof(ligne)) < 0) return -1;
    
    unsigned long long exponent = 0;
    unsigned long long modulus = 0;
    
    if (sscanf(ligne, "%llu %llu", &exponent, &modulus) != 2) return -1;
    
    struct PublicKey cle_recue = {
        .exponent = exponent,
        .modulus = modulus,
    };
    
    // Vérification avant d'enregistrer la clé pour ne pas corrompre le canal de communication
    if (!rsa_is_valid_public_key(cle_recue)) return -1;
    
    *peer_public_key = cle_recue;
    return 0;
}

int channel_send(int socket, struct PublicKey peer_public_key, const char* message) {
    char chiffre[MAX_MESSAGE_SIZE];
    
    if (rsa_encrypt_text(peer_public_key, message, chiffre, sizeof(chiffre)) < 0) return -1;
    
    return send_message(socket, chiffre);
}

int channel_receive(int socket, struct PrivateKey own_private_key, char* message, size_t message_size) {
    char chiffre[MAX_MESSAGE_SIZE];
    
    if (receive_message(socket, chiffre, sizeof(chiffre)) < 0) return -1;
    if (rsa_decrypt_text(own_private_key, chiffre, message, message_size) < 0) return -1;
    
    return 0;
}