#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>

#include "client_array.h"

void ca_init(struct ClientArray* array) {
    array->clients = calloc(2, sizeof(*array->clients));
    if (!array->clients) exit(1);
    
    array->size = 0;
    array->capacity = 2;
    pthread_mutex_init(&array->mutex, NULL);
}

void ca_add(struct ClientArray* array, struct Client client) {
    pthread_mutex_lock(&array->mutex);
    
    if (array->size >= array->capacity) {
        // Doublement de la capacité plutôt qu'une incrémentation unitaire (+1) pour amortir le coût processeur des réallocations successives en cas de connexions massives
        array->clients = realloc(array->clients, 2 * array->capacity * sizeof(*array->clients));
        if (!array->clients) exit(1);
        array->capacity *= 2;
    }
    
    array->clients[array->size] = client;
    array->size++;
    
    pthread_mutex_unlock(&array->mutex);
}

void ca_remove(struct ClientArray* array, int socket) {
    pthread_mutex_lock(&array->mutex);
    
    for (size_t i = 0; i < array->size; ++i) {
        if (array->clients[i].socket == socket) {
            // Remplacement par le dernier élément : évite un décalage coûteux de tous les éléments suivants, car l'ordre des clients dans le tableau n'a aucune importance
            array->clients[i] = array->clients[array->size - 1];
            array->size--;
            break;
        }
    }
    
    pthread_mutex_unlock(&array->mutex);
}

size_t ca_count(struct ClientArray* array) {
    pthread_mutex_lock(&array->mutex);
    size_t taille = array->size;
    pthread_mutex_unlock(&array->mutex);
    return taille;
}

void ca_shutdown_all(struct ClientArray* array) {
    pthread_mutex_lock(&array->mutex);
    
    for (size_t i = 0; i < array->size; ++i) {
        shutdown(array->clients[i].socket, SHUT_RDWR);
    }
    
    pthread_mutex_unlock(&array->mutex);
}

void ca_destroy(struct ClientArray* array) {
    free(array->clients);
    array->clients = NULL;
    array->size = 0;
    array->capacity = 0;
    pthread_mutex_destroy(&array->mutex);
}

void ca_lock(struct ClientArray* array) {
    pthread_mutex_lock(&array->mutex);
}

void ca_unlock(struct ClientArray* array) {
    pthread_mutex_unlock(&array->mutex);
}