
#ifndef SHMHEADER_H
#define	SHMHEADER_H

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

#define TRUE 1
#define FALSE 0
#define BUFFER_SIZE 32
#define HIT "HIT"
#define STAND "STAND"
#define MAX_PLAYERS 4
#define SEGSIZE  sizeof(_ST_SHM)

/*공유메모리 구조체*/
typedef struct {
	int check;
	int check2;
	int finalcheck;
	char data[BUFFER_SIZE];
} _ST_SHM;

void shmClntVersion();
void shmServVersion();


#endif	/* SHMHEADER_H */
