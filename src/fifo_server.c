#include "pipeHeader.h"
#include "blackJack.h"

#define FROM_CLIENT_FILE "./from_client"
#define TO_CLIENT_FILE "./to_client_"

int clientCounter = 0;
char msg[BUFF_SIZE];
char buff[BUFF_SIZE];
char filename[BUFF_SIZE];
char filename2[BUFF_SIZE];

//파이프용
int client[3];
int clientRead[3];
int clientName[3];
int cmd;
int n;
int fd;
int pid;
int check2[2];

/*딜러들의 공유하는 카드덱 전역변수*/
int card_values[52];
int card_suits[52];
int ncard;
/*딜러들은 한 카드덱을 공유*/
pthread_mutex_t card_mutex;
/*카드 패 전역변수*/
int players_hand_values[MAX_PLAYERS][20], dealers_hand_values[MAX_PLAYERS][20];
int players_hand_suits[MAX_PLAYERS][20], dealers_hand_suits[MAX_PLAYERS][20];
int nplayers[MAX_PLAYERS], ndealers[MAX_PLAYERS];

int gnSemID1; /* Semapore Indicator */

void init_cards();
void* play_game_one(void* data);
void* set_shutdown();

/*세마포어 구조체*/
union semun
{
        int val;
        struct semid_ds *buf;
        unsigned short int *array;
};

/*main 스레드*/
void fifoServVersion()
{
	int id;
	/*딜러스레드를 만들 스레드변수*/
	pthread_t threads[MAX_PLAYERS];
	key_t keySem;       /* Semapore Key */
	int count = 1;
	/*세마포어 키*/
	keySem = (key_t)60101;
	char buffer[BUFFER_SIZE];
	union semun sem_union;
	struct sembuf mysem_open = { 0, -1, SEM_UNDO }; // 세마포어 얻기
	struct sembuf mysem_close = { 0, 1, SEM_UNDO };  // 세마포어 돌려주기

	/* 공유 Semapore 세그먼트를 연다 - 필요하면 만든다. */
	if ((gnSemID1 = semget(keySem, 1, IPC_CREAT | IPC_EXCL | 0666)) == -1)
	{
		//  printf("Semapore segment exist - opening as client\n");
		  /* Segment probably already exists - try as a client */
		if ((gnSemID1 = semget(keySem, 0, 0)) == -1)
		{
			perror("semget");
			exit(1);
		}
	}
	else
	{
		// printf("Creating new semapore segment\n");
	}

	/* Signal 등록 */
	(void)signal(SIGINT, (void (*)()) set_shutdown);

	/* Semapore 초기화 */
	sem_union.val = 1;
	semctl(gnSemID1, 0, SETVAL, sem_union);

	/*카드 초기화*/
	init_cards();

	//파이프 -생성 
	mkfifo(FROM_CLIENT_FILE, 0666);

	//파이프 -읽기전용으로 연다
	if ((fd = open(FROM_CLIENT_FILE, O_RDWR)) == -1) {
		printf("[SERVER] open!!!\n");
		exit(1);
	}

	printf("SERVER start!\n\n");
	while (1) {
		memset(buff, 0, BUFF_SIZE);

		//클라이언트 들어오면 받아옴
		if (read(fd, buff, BUFF_SIZE) == -1) {
			break;
		}

		//cmd==3, 각 클라이언트별 pid sscanf
		sscanf(buff, "%d %d|%s", &cmd, &pid, buff);

		if (cmd == 3) {
			printf("[SERVER] client(%d) enter....\n", pid);

			//clientCounter는 들어올때마다 증가, max넘으면 출력
			if (++clientCounter > MAX_CLIENT) {
				printf("[SERVER] client count over!\n");
				break;
				continue;
			}

			//클라이언트 개수 인덱스 배열에 pid 대입
			clientName[clientCounter - 1] = pid;

			//pid 추가한 파일경로로 파이프 생성
			sprintf(filename, "%s%d", TO_CLIENT_FILE, pid);
			mkfifo(filename, 0666);

			sprintf(filename2, "%s%d", FROM_CLIENT_FILE, pid);
			mkfifo(filename2, 0666);

			//파이프 열기
			client[clientCounter - 1] = open(filename, O_RDWR);

			if (semop(gnSemID1, &mysem_open, 1) == -1)
			{
				perror("semop");
				exit(1);
			}

			//쓰레드 생성, 게임 시작, clientCounter도 전달
			pthread_create(&threads[count], NULL, play_game_one, (void*)(intptr_t)clientCounter);


			printf("[SERVER] count: %d\n", count);
			sprintf(buffer, "%d", count);

			count += 1;
			semop(gnSemID1, &mysem_close, 1);
		}
	}
	for (int ii = 1; ii < count; ++ii)
		pthread_join(threads[ii], NULL);
}

/*ctrl+C 로 종료시 공유자원 해제*/
/*서버는 딜러가 항시 대기 하고 있는 것으로 간주하여 따로 종료하지 않는다.*/
/*종료를 위해서는 ctrl+C로 종료*/
void *set_shutdown ()
{
        printf("[SIGNAL] : Got shutdown signal\n");
        semctl( gnSemID1, IPC_RMID, 0 );
        printf("[SIGNAL] : Semapore segment marked for deletion\n");
        exit (1);
}
/*카드덱을 초기화하는 함수*/
void init_cards()
{
	int i;

	for (i = 0; i < 52; ++i)
	{
		card_values[i] = (i % 13) + 1;
		card_suits[i] = i / 13;
	}

	srand(time(NULL));
	/* srand(1); */
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

void* play_game_one(void *data)
{
  int gnSemID2;      /* Semapore Indicator */
  int id = (intptr_t)data;
  int shmId = (intptr_t)data + 60100;
  int semId = (intptr_t)data+1 +60100;
  char buffer[BUFFER_SIZE];
  int nwritten;
  int player_sum, dealer_sum;
  int *player_hand_values = players_hand_values[id], *dealer_hand_values = dealers_hand_values[id];
  int *player_hand_suits = players_hand_suits[id], *dealer_hand_suits = dealers_hand_suits[id];
  int i;

  /* Semapore */
  key_t keySem;       /* Semapore Key */
  union semun sem_union;
  struct sembuf mysem_open  = {0, -1, SEM_UNDO}; // 세마포어 얻기
  struct sembuf mysem_close = {0, 1, SEM_UNDO};  // 세마포어 돌려주기

  /* Semapore 키값을 생성한다. */
  keySem = (key_t)semId;

  /* 공유 Semapore 세그먼트를 연다 - 필요하면 만든다. */
  if((gnSemID2 = semget( keySem, 1, IPC_CREAT | IPC_EXCL | 0666 )) == -1)
  {
   // printf("[SERVER]Semapore segment exist - opening as client\n");
    /* 세그먼트가 이미 존재한다면 - try as a client */
    if( (gnSemID2 = semget( keySem, 0, 0 )) == -1 )
    {
      perror("semget");
      exit(1);
    }
  }
  else
  {
   // printf("[SERVER]Creating new semapore segment\n");
  }

  /* Semapore 초기화 */
  sem_union.val = 1;
  semctl( gnSemID2, 0, SETVAL, sem_union );

  /*첫 카드 뽑기*/
  nplayers[id] = 0;
	ndealers[id] = 0;
  /*딜러들의 카드덱은 공유 - mutex로 raceCondition 제어*/
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

  /******* 임계영역 *******/
  if( semop(gnSemID2, &mysem_open, 1) == -1 )
	{
		perror("semop");
		exit(1);
	}

  //client[id-1]로 카드 줌 )
	write(client[id - 1], buffer, BUFFER_SIZE);

	printf("[SERVER] send to player[%d]: first cardset\n",id);

  semop(gnSemID2, &mysem_close, 1);
  /******* 임계영역 *******/

  /* 첫 카드를 주고서*/
  printf("\n");
  printf("[SERVER] Player[%d] Hand: ",id );
  display_state(player_hand_values, player_hand_suits, nplayers[id]);
  printf("[SERVER] Dealer Hand with player[%d]: ",id);
  display_state(dealer_hand_values, dealer_hand_suits, ndealers[id]);

  //HIT STAND 받을 파이프 open
  clientRead[id - 1] = open(filename2, O_RDWR);

  while (TRUE) {
	  sleep(1);
	  //HIT / STAND 받음
	  read(clientRead[id - 1], buffer, BUFFER_SIZE);
	  printf("[SERVER] I received from player[%d]: %s\n", id, buffer);

	  //HIT일때
	  if (strcmp(buffer, HIT) == 0)
	  {
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

		  //새로운 카드 클라이언트로 보냄
		  write(client[id - 1], buffer, BUFFER_SIZE);
		  printf("[SERVER] I send to player[%d]: %s\n", id, buffer);

		  check2[0] = 1;
		  buffer[0] = '\0';
		  semop(gnSemID2, &mysem_close, 1);

		  /* 플레이어 결과가 21이 넘을 경우*/
		  if (calc_sum(player_hand_values, nplayers[id]) > 21)
		  {
			  printf("[SERVER] Player[%d] busted. Dealer wins.\n", id);
			  return NULL;
		  }
		  /* 플레이어 결과가 21일 경우 이기기 때문에 break */
		  else if (calc_sum(player_hand_values, nplayers[id]) == 21)
			  break;
	  } //여기까지 HIT 처리

	  //STAND 일때 - break
	  else if (strcmp(buffer, STAND) == 0) {
		  check2[0] = 1;
		  semop(gnSemID2, &mysem_close, 1);
		  break;
	  }
  }
  i=0;
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
  printf("[SERVER] Player[%d] Hand: ",id);
  display_state(player_hand_values, player_hand_suits, nplayers[id]);
  printf("[SERVER] Dealer Hand with Player[%d]: ",id);
  display_state(dealer_hand_values, dealer_hand_suits, ndealers[id]);

  if( semop(gnSemID2, &mysem_open, 1) == -1 )
  {
          perror("semop");
          exit(1);
  }
  //카드 보냄
  write(client[id - 1], buffer, BUFFER_SIZE);


  player_sum = calc_sum(player_hand_values, nplayers[id]);
  dealer_sum = calc_sum(dealer_hand_values, ndealers[id]);

  //결과
  if (dealer_sum > 21)
    printf("\n[SERVER] Dealer busted! Player[%d] wins!\n", id);
  else if (player_sum == dealer_sum)
    printf("\n[SERVER] Player[%d] and dealer have the SAME score. It's a push!\n", id);
  else if (player_sum < dealer_sum)
    printf("\n[SERVER] Dealer has a higher score than player[%d]. Dealer wins!\n", id);
  else
    printf("\n[SERVER] Player[%d] has a higher score. Player wins!\n", id);
}
