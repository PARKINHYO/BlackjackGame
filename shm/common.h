
#ifndef COMMON_H
#define	COMMON_H

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

/*게임에 필요한 카드*/
const char* suits[] = {"spades", "hearts", "diamonds", "clubs"};
const char* values[] = {"dummy", "Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"};
const char suit_codes[] = {'S', 'H', 'D', 'C'};
const char value_codes[] = {'0', 'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'};

/*공유메모리 구조체*/
typedef struct {
	int check;
	int check2;
	int finalcheck;
	char data[BUFFER_SIZE];
} _ST_SHM;

/*에러 print 함수*/
void error(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}
/*suit를 받아오는 함수*/
int get_suit_id(char suit)
{
	unsigned i;
	for (i = 0; i < sizeof (suit_codes); ++i)
		if (suit == suit_codes[i])
			return i;
	return -1;
}
/*value(카드)를 받아오는 함수*/
int get_value_id(char value)
{
	unsigned i;
	for (i = 0; i < sizeof (value_codes); ++i)
		if (value == value_codes[i])
			return i;
	return -1;
}
/*카드 패를 합산하는 함수*/
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

/*카드패 화면 출력 함수*/
int display_state(int hand_values[], int hand_suits[], int ncards)
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
	if (sum > 21){
		printf("; BUSTED");
		printf("\n");
		return 1;
		}
	else if (sum == 21){
		printf("\n");
		return 1;
	}
	printf("\n");
	return 0;
}

#endif	/* COMMON_H */
