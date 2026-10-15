#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

#define MAX_MESSAGE_SIZE 1024

int send_message(int socket, const char* message);
int receive_message(int socket, char* message, size_t message_size);

#endif