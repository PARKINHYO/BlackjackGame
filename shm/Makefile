INCLUDE_DIR=./../
CC = gcc
default:
	gcc -I${INCLUDE_DIR} -o client shm_client.c -lrt -lpthread
	gcc -I${INCLUDE_DIR} -o server shm_server.c -lrt -lpthread

clean:
	rm -f client server
