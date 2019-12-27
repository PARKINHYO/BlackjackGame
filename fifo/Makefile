INCLUDE_DIR=./../

default:
	gcc -I${INCLUDE_DIR} -o client fifo_client.c -lpthread
	gcc -I${INCLUDE_DIR} -o server fifo_server.c -lpthread

clean:
	rm -f client server
