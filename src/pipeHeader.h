#ifndef PIPEHEADER_H
#define	PIPEHEADER_H

#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/stat.h> 
#include <fcntl.h> 
#include <signal.h>
s
#define TRUE 1
#define FALSE 0
#define BUFFER_SIZE 32
#define BUFF_SIZE 32
#define HIT "HIT"
#define STAND "STAND"
#define MAX_PLAYERS 5
#define MAX_CLIENT 3

typedef struct {
	int check;
	int check2;
	int finalcheck;
	char data[BUFFER_SIZE];
} _ST_SHM;

void fifoServVersion();
void fifoClntVersion();

#endif	/* PIPEHEADER_H */
