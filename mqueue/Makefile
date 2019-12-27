INCLUDE_DIR=./../

default:
	gcc -I${INCLUDE_DIR} -o client mqueue_client.c -lpthread
	gcc -I${INCLUDE_DIR} -o server mqueue_server.c -lpthread

clean:
	rm -f client server
