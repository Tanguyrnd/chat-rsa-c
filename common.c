#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "common.h"

static int send_all(int socket, const char* data, size_t length) {
    size_t total = 0;
    
    while (total < length) {
        // MSG_NOSIGNAL : Empêche le noyau Linux de tuer brutalement le processus (avec SIGPIPE) si le destinataire a fermé la connexion de son côté
        ssize_t envoye = send(socket, data + total, length - total, MSG_NOSIGNAL);
        if (envoye < 0) return -1;
        total += (size_t)envoye;
    }
    
    return 0;
}

int send_message(int socket, const char* message) {
    char ligne[MAX_MESSAGE_SIZE + 1];
    int taille = snprintf(ligne, sizeof(ligne), "%s\n", message);
    
    if (taille < 0 || (size_t)taille >= sizeof(ligne)) return -1;
    
    return send_all(socket, ligne, (size_t)taille);
}

int receive_message(int socket, char* message, size_t message_size) {
    size_t taille = 0;
    
    while (1) {
        char c = '\0';
        // Lecture d'un octet à la fois : TCP est un flux continu. Lire par blocs risquerait d'aspirer le début du message suivant sans posséder de système de mise en cache pour le conserver
        ssize_t recu = recv(socket, &c, 1, 0);
        
        if (recu <= 0) return -1;
        if (c == '\n') break;
        
        if (taille + 1 >= message_size) return -1;
        
        message[taille] = c;
        taille++;
    }
    
    message[taille] = '\0';
    return 0;
}