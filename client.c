#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"
#include "rsa.h"
#include "rsa_text.h"
#include "secure_channel.h"

struct Client {
    int socket;
    atomic_bool should_stop;
    struct PublicKey server_public_key;
    struct PrivateKey own_private_key;
};

void send_message_loop(struct Client* client) {
    char message[RSA_MAX_PLAIN_LENGTH + 1];
    
    while (!client->should_stop) {
        if (!fgets(message, sizeof(message), stdin)) return;
        
        size_t taille = strcspn(message, "\n");
        message[taille] = '\0';
        
        if (!message[0]) continue;
        if (channel_send(client->socket, client->server_public_key, message) < 0) return;
    }
}

void receive_message_loop(struct Client* client) {
    char message[RSA_MAX_PLAIN_LENGTH + 1];
    
    while (!client->should_stop) {
        if (!channel_receive(client->socket, client->own_private_key, message, sizeof(message))) {
            printf("Message: %s\n", message);
        } else {
            client->should_stop = true;
        }
    }
}

void* receive_messages_thread(void* client) {
    receive_message_loop((struct Client*)client);
    return NULL;
}

int main(void) {
    rsa_seed_random();
    
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) exit(1);
    
    struct sockaddr_in adresse = {0};
    adresse.sin_family = AF_INET;
    adresse.sin_addr.s_addr = inet_addr("127.0.0.1");
    adresse.sin_port = htons(8989);
    
    if (connect(client_socket, (struct sockaddr*)&adresse, sizeof(adresse)) < 0) exit(1);
    
    printf("Connected to server\n");
    
    struct Client client = {
        .socket = client_socket,
        .should_stop = false,
    };
    
    struct PublicKey cle_publique;
    rsa_generate_keys(&cle_publique, &client.own_private_key);
    
    if (channel_exchange_keys(client_socket, cle_publique, &client.server_public_key) < 0) {
        close(client_socket);
        exit(1);
    }
    
    pthread_t thread;
    pthread_create(&thread, NULL, receive_messages_thread, &client);
    
    send_message_loop(&client);
    
    client.should_stop = true;
    // Coupure de la connexion réseau avant le join : force la fonction bloquante recv() du thread secondaire à se terminer instantanément
    shutdown(client_socket, SHUT_RDWR);
    pthread_join(thread, NULL);
    close(client_socket);
    
    return 0;
}