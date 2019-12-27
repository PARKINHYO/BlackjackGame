#include "common.h"

#define KEY_VALUE_MAIN (key_t)60300
#define KEY_VALUE_MAIN2 (key_t)60301
#define PERM 0666

void* play_game();
void* set_shutdown();
void* send_msg();
void* recv_msg();

int msgid_Send_Main;
int msgid_Recv_Main;

pid_t pid;
int nwritten;
pthread_t threads[MAX_PLAYERS];

int my_sum;
int dealer_sum;

int check = 0;
int check2 = 0;
int finalcheck = 0;

// 메세지큐 해제...
void *set_shutdown()
{
	printf("[SIGNAL] : Got shutdown signal\n");
	msgctl(msgid_Send_Main, IPC_RMID, 0);
	printf("[SIGNAL] : Message passing queue marked for deletion\n");
	msgctl(msgid_Recv_Main, IPC_RMID, 0);
	printf("[SIGNAL] : Message passing queue marked for deletion\n");
	exit(1);
}

void* send_msg() {
	msg buf;
	buf.type = pid;

	char buffer[BUFFER_SIZE];
	int choice;

	while (1) {
		if(finalcheck) {break;}
		if(check2 == 1){
			printf("\n");
			printf("1. Hit\n");
			printf("2. Stand\n");
			printf("Please choose 1 or 2: ");
			fflush(stdout);
			scanf("%d", &choice);

			if (choice == 1)
			{
				strcpy(buffer, HIT);
				printf("Sending: %s\n", buffer);

				strncpy(buf.data, buffer, BUFFER_SIZE);
				msgsnd(msgid_Send_Main, (void*)&buf, sizeof(msg), 0);
				buffer[0]='\0';
				check2 =2;

			}
			else if (choice == 2)
			{
				strcpy(buffer, STAND);
				printf("Sending: %s\n", buffer);
				strncpy(buf.data, buffer, BUFFER_SIZE);
				msgsnd(msgid_Send_Main, (void*)&buf, sizeof(msg), 0);
				buffer[0]='\0';	
				check2 = 0;
				check = 0;
				finalcheck = 1;
				break;
			}
			else
				printf("Unrecognized choice. Choose again.\n");
		}
	}
}


void* recv_msg() {
	msg buf;
	buf.type = pid;

	char buffer[BUFFER_SIZE];

	check =1;
	int my_hand_values[20], dealer_hand_values[20];
	int my_hand_suits[20], dealer_hand_suits[20];
	int nmy = 0, ndealer = 0;
	buf.type = pid;

	while (1) {
		if(check == 1){

			printf("pid: %d\n\n", pid);
			msgrcv(msgid_Recv_Main, (void*)&buf, sizeof(msg), pid, 0);
			strncpy(buffer, buf.data, BUFFER_SIZE);
			printf("%s\n", buffer);

			my_hand_values[0] = get_value_id(buffer[0]);
			my_hand_suits[0] = get_suit_id(buffer[1]);
			my_hand_values[1] = get_value_id(buffer[2]);
			my_hand_suits[1] = get_suit_id(buffer[3]);
			dealer_hand_values[0] = get_value_id(buffer[4]);
			dealer_hand_suits[0] = get_suit_id(buffer[5]);
			nmy = 2;
			ndealer = 1;

			my_sum = calc_sum(my_hand_values, nmy);

			printf("\n");
			// 카드덱과 합산값 화면출력
			printf("My Hand: ");
			display_state(my_hand_values, my_hand_suits, nmy);
			printf("Dealer Hand: ");
			display_state(dealer_hand_values, dealer_hand_suits, ndealer);

			if (my_sum > 21)
			{
				printf("\nI'm busted! I lose!\n");
				return 0;
			}

			check = 0;
			check2 = 1;
			buffer[0]='\0';
		}
		if(check2 == 2){
			printf("buffer: %s\n", buffer);
			msgrcv(msgid_Recv_Main, (void*)&buf, sizeof(msg), pid, 0);
			printf("buf.data: %s\n", buf.data);
			strncpy(buffer, buf.data, BUFFER_SIZE);
			printf("I received: %s\n", buffer);
			my_hand_values[nmy] = get_value_id(buffer[0]);
			my_hand_suits[nmy] = get_suit_id(buffer[1]);
			++nmy;

			printf("\n");
			printf("My Hand: ");
			display_state(my_hand_values, my_hand_suits, nmy);
			printf("Dealer Hand: ");
			display_state(dealer_hand_values, dealer_hand_suits, ndealer);
			my_sum = calc_sum(my_hand_values, nmy);
			if(my_sum>21){
				printf("\nI'm busted! I lose!\n");
				return 0;
			}

			check2 = 1;
		}

		if(finalcheck == 1){
			finalcheck = 0;
			unsigned i;
			msgrcv(msgid_Recv_Main, (void*)&buf, sizeof(msg), pid, 0);
			strncpy(buffer, buf.data, BUFFER_SIZE);

			printf("I received: %s\n", buffer);

			for (i = 0; i < strlen(buffer); i += 2)
			{
				dealer_hand_values[ndealer] = get_value_id(buffer[i]);
				dealer_hand_suits[ndealer] = get_suit_id(buffer[i + 1]);
				++ndealer;
			}

			printf("\n");
			printf("My Hand: ");
			display_state(my_hand_values, my_hand_suits, nmy);
			printf("Dealer Hand: ");
			display_state(dealer_hand_values, dealer_hand_suits, ndealer);

			my_sum = calc_sum(my_hand_values, nmy);
			dealer_sum = calc_sum(dealer_hand_values, ndealer);

			if (dealer_sum > 21)
				printf("\nDealer busted! I win!\n");
			else if (my_sum == dealer_sum)
				printf("\nMe and the dealer have the same score. It's a push!\n");
			else if (my_sum < dealer_sum)
				printf("\nDealer has a higher score. I lose!\n");
			else
				printf("\nI have a higher score. I win!\n");

			return 0;

		}
	}
	return 0;
}
void* play_game() {

	// recv_msg, send_msg 스레드 생성...
	pthread_create(&threads[1], NULL, recv_msg, NULL);
	pthread_create(&threads[2], NULL, send_msg, NULL);
	int tid;
	// 스레드들 끝날때까지 대기후 삭제...
	for (tid = 1; tid < 2; ++tid)
		pthread_join(threads[tid], NULL);

}

int main() {

	char buffer[BUFFER_SIZE];
	int count = 0;
	pid = getpid();
	msg buf;

	msgid_Send_Main = msgget(KEY_VALUE_MAIN, IPC_CREAT | PERM);
	msgid_Recv_Main = msgget(KEY_VALUE_MAIN2, IPC_CREAT | PERM);

	// Signal 등록 ...
	(void)signal(SIGINT, (void(*)()) set_shutdown);

	printf("input data ==> ");
	fgets(buffer, BUFFER_SIZE, stdin);

	buf.type = 200;
	memset(buf.data, 0, sizeof(buf.data));
	strncpy(buf.data, buffer, BUFFER_SIZE);
	msgsnd(msgid_Send_Main, (void*)&buf, sizeof(msg), 0);

	// 쓴 데이터가 ‘quit’이면 while 문 벗어남...
	if (!strncmp(buf.data, "quit", 4)) {
		return 0;
	}

	// play_game 스레드 진입...
	pthread_create(&threads[0], NULL, play_game, NULL);
	// 스레드 종료 대기후 삭제...
	pthread_join(threads[0], NULL);
	printf("game end!\n");
	return 0;

}

