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

#define TRUE 1
#define FALSE 0
#define BUFFER_SIZE 32
#define BUFF_SIZE 32
#define HIT "HIT"
#define STAND "STAND"
#define MAX_PLAYERS 5
#define MAX_CLIENT 3

const char* suits[] = {"spades", "hearts", "diamonds", "clubs"};
const char* values[] = {"dummy", "Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"};
const char suit_codes[] = {'S', 'H', 'D', 'C'};
const char value_codes[] = {'0', 'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'};
typedef struct {
	int check;
	int check2;
	int finalcheck;
	char data[BUFFER_SIZE];
} _ST_SHM;

void error(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

int get_suit_id(char suit)
{
	unsigned i;
	for (i = 0; i < sizeof (suit_codes); ++i)
		if (suit == suit_codes[i])
			return i;
	return -1;
}

int get_value_id(char value)
{
	unsigned i;
	for (i = 0; i < sizeof (value_codes); ++i)
		if (value == value_codes[i])
			return i;
	return -1;
}

int calc_sum(const int hand_values[], int ncards)
{
	int i;
	int sum = 0;

	for (i = 0; i < ncards; ++i)
		if (hand_values[i] < 10)
			sum += hand_values[i];
		else
			sum += 10;

	for (i = 0; i < ncards; ++i)
		if (hand_values[i] == 1)
			if (sum + 10 <= 21)
				sum += 10;

	return sum;
}

void display_state(int hand_values[], int hand_suits[], int ncards)
{
	int i;
	int sum = calc_sum(hand_values, ncards);
	for (i = 0; i < ncards; ++i)
	{
		if (i > 0)
			printf(", ");
		printf("%s-%s", values[hand_values[i]], suits[hand_suits[i]]);
	}
	printf(" Sum: %d", sum);
	if (sum > 21)
		printf("; BUSTED");
	printf("\n");
}


#endif	/* PIPEHEADER_H */
