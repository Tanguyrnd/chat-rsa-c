CFLAGS = -Wall -Wextra -Wpedantic -std=c23

all: server client

common.o: common.c common.h
	gcc -c common.c

number_theory.o: number_theory.c number_theory.h
	gcc -c number_theory.c

rsa.o: rsa.c rsa.h number_theory.h
	gcc -c rsa.c

rsa_text.o: rsa_text.c rsa_text.h rsa.h
	gcc -c rsa_text.c

secure_channel.o: secure_channel.c secure_channel.h common.h rsa.h rsa_text.h
	gcc -c secure_channel.c

client_array.o: client_array.c client_array.h rsa.h
	gcc -c client_array.c

server.o: server.c client_array.h rsa.h rsa_text.h secure_channel.h
	gcc -c server.c

client.o: client.c rsa.h rsa_text.h secure_channel.h
	gcc -c client.c

server: server.o common.o client_array.o number_theory.o rsa.o rsa_text.o secure_channel.o
	gcc -o server server.o common.o client_array.o number_theory.o rsa.o rsa_text.o secure_channel.o

client: client.o common.o number_theory.o rsa.o rsa_text.o secure_channel.o
	gcc -o client client.o common.o number_theory.o rsa.o rsa_text.o secure_channel.o

.PHONY: clean
clean:
	-rm -f server client *.o