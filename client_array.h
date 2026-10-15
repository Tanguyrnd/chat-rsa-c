#ifndef CLIENT_ARRAY_H
#define CLIENT_ARRAY_H

#include <pthread.h>
#include <stddef.h>

#include "rsa.h"

struct Client {
    int socket;
    pthread_t thread;
    struct PublicKey public_key;
};

struct ClientArray {
    struct Client* clients;
    size_t size;
    size_t capacity;
    pthread_mutex_t mutex;
};

void ca_init(struct ClientArray* array);
void ca_add(struct ClientArray* array, struct Client client);
void ca_remove(struct ClientArray* array, int socket);
size_t ca_count(struct ClientArray* array);
void ca_shutdown_all(struct ClientArray* array);
void ca_destroy(struct ClientArray* array);
void ca_lock(struct ClientArray* array);
void ca_unlock(struct ClientArray* array);

#endif