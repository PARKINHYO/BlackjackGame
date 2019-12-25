main: src/main.o  src/fifo_client.o src/fifo_server.o src/mqueue_client.o src/mqueue_server.o src/shm_client.o src/shm_server.o
	gcc src/fifo_client.o src/fifo_server.o src/mqueue_client.o src/mqueue_server.o src/shm_client.o src/shm_server.o src/main.o ./blackjack -02 -Wall -Wshadow -lpthread
	rm -rf src/*.o src/*.gch
main.o: src/main.c
	gcc src/main.c -c

fifo_client.o: src/fifo_client.c src/pipeHeader.h
	gcc src/fifo_client.c src/pipeHeader.h -c -lpthread

fifo_server.o: src/fifo_server.c src/pipeHeader.h
	gcc src/fifo_srver.c src/pipeHeader.h -c -lpthread

mqueue_client.o: src/mqueue_client.c src/mqueueHeader.h
	gcc src/mqueue_client.c src/mqueuHeader.h -c -lpthread

mqueue_server.o: src/mqueu_server.c src/mqeueHeader.h
	gcc src/mqueu_server.c src/mqueueHeader.h -c -lpthread

shm_client.o: src/shm_client.c src/shmHeader.h
	gcc src/shm_client.c src/shmHeader.h -c -lpthread

shm_server.o: src/shm_server.c src/shmHeader.h
	gcc src/shm_server.c src/shmHeader.h -c -lpthread
