#include "mqueueHeader.h"
#include "blackJack.h"

#define KEY_VALUE_MAIN (key_t)60300
#define KEY_VALUE_MAIN2 (key_t)60301
#define PERM 0666

void* play_game_one(void* data);
void* set_shutdown();
void init_cards();

int card_values[52];
int card_suits[52];
int ncard;
pthread_mutex_t card_mutex;
int players_hand_values[MAX_PLAYERS][20], dealers_hand_values[MAX_PLAYERS][20];
int players_hand_suits[MAX_PLAYERS][20], dealers_hand_suits[MAX_PLAYERS][20];
int nplayers[MAX_PLAYERS], ndealers[MAX_PLAYERS];

int msgid_Send_Main;
int msgid_Recv_Main;

void mqServVersion()
{
	int id;
	struct msqid_ds msq_status;

	pthread_t threads[MAX_PLAYERS];
	msg buf;
	int count = 0;
	init_cards();
	int i;

	// 메세지큐 생성...
	msgid_Send_Main = msgget(KEY_VALUE_MAIN2, IPC_CREAT | PERM);
	msgid_Recv_Main = msgget(KEY_VALUE_MAIN, IPC_CREAT | PERM);

	msgctl(msgid_Recv_Main, IPC_STAT, &msq_status);

	// signal 생성...
	(void)signal(SIGINT, (void(*)()) set_shutdown);

	for (i = 0; i < 10; i++) {
		msgrcv(msgid_Recv_Main, (void*)&buf, sizeof(msg), 200, 0);
		printf("start! : %s", buf.data); // start 정상적으로 받습니다.
		if (strncmp(buf.data, "start", 5) == 0) {
			printf("correct!\n");
			pthread_create(&threads[count], NULL, play_game_one, (void *)(intptr_t)count); // 게임 실행 스레드 생성합니다.
			count +=1;
		}
	}
}

void* play_game_one(void *data)
{

	struct msqid_ds msq_status;

	int id = (intptr_t)data;
	char buffer[BUFFER_SIZE];
	int nwritten;
	int player_sum, dealer_sum;
	int* player_hand_values = players_hand_values[id], *dealer_hand_values = dealers_hand_values[id];
	int* player_hand_suits = players_hand_suits[id], *dealer_hand_suits = dealers_hand_suits[id];
	int i;
	msg buf;

	msgctl(msgid_Recv_Main, IPC_STAT, &msq_status);
	buf.type = msq_status.msg_lspid;
	printf("%ld\n", buf.type);

	nplayers[id] = 0;
	ndealers[id] = 0;

	pthread_mutex_lock(&card_mutex);
	{
		for (i = 0; i < 2; ++i)
		{
			player_hand_values[nplayers[id]] = card_values[ncard];
			player_hand_suits[nplayers[id]] = card_suits[ncard];
			++ncard;
			++nplayers[id];
		}

		dealer_hand_values[ndealers[id]] = card_values[ncard];
		dealer_hand_suits[ndealers[id]] = card_suits[ncard];
		++ncard;
		++ndealers[id];
	}
	pthread_mutex_unlock(&card_mutex);

	buffer[0] = value_codes[player_hand_values[0]];
	buffer[1] = suit_codes[player_hand_suits[0]];
	buffer[2] = value_codes[player_hand_values[1]];
	buffer[3] = suit_codes[player_hand_suits[1]];
	buffer[4] = value_codes[dealer_hand_values[0]];
	buffer[5] = suit_codes[dealer_hand_suits[0]];
	buffer[6] = 0;

	strncpy(buf.data, buffer, BUFFER_SIZE);
	msgsnd(msgid_Send_Main, (void*)&buf, sizeof(msg), 0);
	printf("send: first cardset\n");

	//첫번째 카드를 뿌려주고 난뒤...
	printf("\n");
	printf("Player 1 Hand: ");
	display_state(player_hand_values, player_hand_suits, nplayers[id]);
	printf("Dealer Hand with player 1: ");
	display_state(dealer_hand_values, dealer_hand_suits, ndealers[id]);
	int k;

	while(1) {
		msgrcv(msgid_Recv_Main, (void*)&buf, sizeof(msg), buf.type, 0);
		printf("buf.data: %s\n", buf.data);

		strncpy(buffer, buf.data, BUFFER_SIZE);
		printf("I received from player 1: %s\n", buffer);
		strncpy(buf.data, "\0", BUFFER_SIZE);

		//hit 결과 전송...
		if (strcmp(buffer, HIT) == 0)
		{
			//뮤텍스로 상호배제...
			pthread_mutex_lock(&card_mutex);
			{
				player_hand_values[nplayers[id]] = card_values[ncard];
				player_hand_suits[nplayers[id]] = card_suits[ncard];

				buffer[0] = value_codes[player_hand_values[nplayers[id]]];
				buffer[1] = suit_codes[player_hand_suits[nplayers[id]]];
				buffer[2] = 0;

				++ncard;
				++nplayers[id];
			}
			pthread_mutex_unlock(&card_mutex);

			strncpy(buf.data, buffer, BUFFER_SIZE);
			msgsnd(msgid_Send_Main, (void*)&buf, sizeof(msg), 0);
			printf("buf.data: %s\n", buf.data);
			printf("I send to player 1: %s\n", buffer);

			buffer[0] = '\0';

			// 플레이어 결과가 21이 넘을 경우
			if (calc_sum(player_hand_values, nplayers[id]) > 21)
			{
				printf("Player 1 busted. Dealer wins.\n");
				return NULL;
			}
			// 플레이어 결과가 21일 경우 이기기 때문에 break
			else if (calc_sum(player_hand_values, nplayers[id]) == 21)
				break;
		}
		// 플레이어의 signal이 STAND인 경우 stop
		else if (strcmp(buffer, STAND) == 0) {
			break;
		}
	}
	i = 0;
	while (calc_sum(dealer_hand_values, ndealers[id]) < 17)
	{
		dealer_hand_values[ndealers[id]] = card_values[ncard];
		dealer_hand_suits[ndealers[id]] = card_suits[ncard];
		buffer[i++] = value_codes[dealer_hand_values[ndealers[id]]];
		buffer[i++] = suit_codes[dealer_hand_suits[ndealers[id]]];
		++ncard;
		++ndealers[id];
	}
	buffer[i] = 0;


	printf("\n");
	printf("Player 1 Hand: ");
	display_state(player_hand_values, player_hand_suits, nplayers[id]);
	printf("Dealer 1 Hand: ");
	display_state(dealer_hand_values, dealer_hand_suits, ndealers[id]);

	strncpy(buf.data, buffer, BUFFER_SIZE);
	msgsnd(msgid_Send_Main, (void*)&buf, sizeof(msg), 0);

	player_sum = calc_sum(player_hand_values, nplayers[id]);
	dealer_sum = calc_sum(dealer_hand_values, ndealers[id]);

	if (dealer_sum > 21)
		printf("\nDealer busted! Player %d wins!\n", id);
	else if (player_sum == dealer_sum)
		printf("\nPlayer %d and dealer have the same score. It's a push!\n", id);
	else if (player_sum < dealer_sum)
		printf("\nDealer has a higher score than player %d. Dealer wins!\n", id);
	else
		printf("\nPlayer %d has a higher score. Player wins!\n", id);

	return 0;
}

void init_cards()
{
	int i;

	for (i = 0; i < 52; ++i)
	{
		card_values[i] = (i % 13) + 1;
		card_suits[i] = i / 13;
	}

	srand(time(NULL));
	// srand...
	for (i = 0; i < 52; ++i)
	{
		int cv, cs;
		int j = rand() % 52;

		cv = card_values[i];
		card_values[i] = card_values[j];
		card_values[j] = cv;

		cs = card_suits[i];
		card_suits[i] = card_suits[j];
		card_suits[j] = cs;
	}

	ncard = 0;
}

//메세지큐 해제...
void *set_shutdown()
{
	printf("[SIGNAL] : Got shutdown signal\n");
	msgctl(msgid_Send_Main, IPC_RMID, 0);
	printf("[SIGNAL] : Message passing queue marked for deletion\n");
	msgctl(msgid_Recv_Main, IPC_RMID, 0);
	printf("[SIGNAL] : Message passing queue marked for deletion\n");
	exit(1);
}



