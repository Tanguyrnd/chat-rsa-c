#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client_array.h"
#include "common.h"
#include "rsa.h"
#include "rsa_text.h"
#include "secure_channel.h"

static volatile sig_atomic_t should_stop = 0;

struct ThreadData {
    int client_socket;
    struct ClientArray* client_array;
    struct PublicKey server_public_key;
    struct PrivateKey server_private_key;
};

void signal_handler(int signal) {
    (void)signal;
    should_stop = 1;
}

void broadcast_message(struct ClientArray* client_array, int sender_socket, const char* message) {
    // Verrouillage pour éviter qu'un thread supprime un client pendant le parcours du tableau
    ca_lock(client_array);
    
    for (size_t i = 0; i < client_array->size; ++i) {
        struct Client* client = &client_array->clients[i];
        if (client->socket != sender_socket) {
            channel_send(client->socket, client->public_key, message);
        }
    }
    
    ca_unlock(client_array);
}

void receive_message_loop(const struct ThreadData* data) {
    char message[RSA_MAX_PLAIN_LENGTH + 1];
    
    while (1) {
        if (channel_receive(data->client_socket, data->server_private_key, message, sizeof(message)) < 0) return;
        printf("Message: %s\n", message);
        broadcast_message(data->client_array, data->client_socket, message);
    }
}

void* client_thread_main(void* thread_data) {
    struct ThreadData* data = (struct ThreadData*)thread_data;
    struct PublicKey cle_client;
    
    if (channel_exchange_keys(data->client_socket, data->server_public_key, &cle_client) < 0) {
        close(data->client_socket);
        free(data);
        return NULL;
    }
    
    struct Client client = {
        .socket = data->client_socket,
        .thread = pthread_self(),
        .public_key = cle_client,
    };
    
    ca_add(data->client_array, client);
    receive_message_loop(data);
    ca_remove(data->client_array, data->client_socket);
    
    close(data->client_socket);
    free(data);
    return NULL;
}

void start_client_thread(int client_socket, struct ClientArray* client_array, struct PublicKey server_public_key, struct PrivateKey server_private_key) {
    // Allocation dynamique : une variable locale serait écrasée à la prochaine itération de accept()                     
    struct ThreadData* data = malloc(sizeof(*data));
    if (!data) {
        close(client_socket);
        return;
    }
    
    data->client_socket = client_socket;
    data->client_array = client_array;
    data->server_public_key = server_public_key;
    data->server_private_key = server_private_key;
    
    pthread_t thread;
    if (pthread_create(&thread, NULL, client_thread_main, data) != 0) {
        close(client_socket);
        free(data);
        return;
    }
    
    // Détachement du thread pour que l'OS libère automatiquement ses ressources
    pthread_detach(thread);
}

int main(void) {
    rsa_seed_random();
    
    struct PublicKey cle_publique;
    struct PrivateKey cle_privee;
    rsa_generate_keys(&cle_publique, &cle_privee);
    
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) exit(1);
    
    int opt = 1;
    // SO_REUSEADDR permet de relancer le serveur immédiatement sans attendre le timeout TCP du port
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) exit(1);
    
    struct sockaddr_in adresse = {0};
    adresse.sin_family = AF_INET;
    adresse.sin_addr.s_addr = INADDR_ANY;
    adresse.sin_port = htons(8989);
    
    if (bind(server_socket, (struct sockaddr*)&adresse, sizeof(adresse)) < 0) exit(1);
    if (listen(server_socket, SOMAXCONN) < 0) exit(1);
    
    printf("Server listening...\n");
    
    struct ClientArray clients;
    ca_init(&clients);
    
    struct sigaction action = {0};
    action.sa_handler = &signal_handler;
    sigaction(SIGINT, &action, NULL);
    
    while (!should_stop) {
        struct sockaddr_in adresse_client = {0};
        socklen_t taille_adresse = sizeof(adresse_client);
        int client_socket = accept(server_socket, (struct sockaddr*)&adresse_client, &taille_adresse);
        
        if (client_socket >= 0) {
            printf("Client connected from %s\n", inet_ntoa(adresse_client.sin_addr));
            start_client_thread(client_socket, &clients, cle_publique, cle_privee);
        }
    }
    
    ca_shutdown_all(&clients);
    
    // Attente pour laisser le temps aux threads existants de se terminer et se retirer
    while (ca_count(&clients) > 0) {
        usleep(10000);
    }
    
    ca_destroy(&clients);
    close(server_socket);
    
    return 0;
}