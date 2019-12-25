
#ifndef MQUEUEHEADER_H
#define	MQUEUEHEADER_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#define BUFFER_SIZE 32
#define HIT "HIT"
#define STAND "STAND"
#define MAX_PLAYERS 10

//카드 덱들...
typedef struct MSG {
	long type;
	char data[BUFFER_SIZE];
}msg;

int mqClntVersion();
void mqServVersion();

#endif /* MQUEUEHEADER_H */
