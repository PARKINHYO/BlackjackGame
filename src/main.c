#include <stdio.h>
#include <stdlib.h>
#include "pipeHeader.h"
#include "mqueueHeader.h"
#include "shmHeader.h"

void showExe(char*);

int main(int argc, char *argv[]){
	if(argc>2)
		showExe(argv[0]);
	/* fifo server: -fs */
	else if(strcmp(argv[1], "-fs")==0){
		printf("------------ fifo server -------------\n");
		fifoServVersion();
	}
	/* fifo client: -fc */
	else if(strcmp(argv[1], "-fc")==0){
		printf("------------ fifo client -------------\n");
		fifoClntVersion();
	}
	/* message queue server: -ms */
	else if(strcmp(argv[1], "-ms")==0){
		printf("---------- messagequeue server ------------\n");
		mqServVersion();
	}
	/* message queue client: -mc */
	else if(strcmp(argv[1], "-mc")==0){
		printf("---------- messagequeue client ------------\n");
		mqClntVersion();
	}
	/* shared memory server: -ss */
	else if(strcmp(argv[1], "-ss")==0){
		printf("----- Shared Memory server -----\n");
		shmServVersion();
	}
	/* shared memory client: -sc */
	else if(strcmp(argv[1], "-sc")==0){
		printf("------- shared memory client ------\n");
		shmClntVersion();
	}	
	else
		showExe(argv[0]);
	return 0;
}

void showExe(char *s){
	printf("%s [option]\n", s);
	printf("no option: Play the game with normal version. \n");
	printf(" -fs: fifo server \n");
	printf(" -fc: fifo client \n");
	printf(" -ms: message queue server \n");
	printf(" -mc: message queue client \n");
	printf(" -ss: shared memory server \n");
	printf(" -sc: shared memory client \n");
}
