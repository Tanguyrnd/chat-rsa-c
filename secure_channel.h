#ifndef SECURE_CHANNEL_H
#define SECURE_CHANNEL_H

#include <stddef.h>

#include "rsa.h"

int channel_exchange_keys(int socket, struct PublicKey own_public_key, struct PublicKey* peer_public_key);
int channel_send(int socket, struct PublicKey peer_public_key, const char* message);
int channel_receive(int socket, struct PrivateKey own_private_key, char* message, size_t message_size);

#endif