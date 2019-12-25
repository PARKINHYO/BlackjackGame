#include "shmHeader.h"
#include "blackJack.h"

int gnShmID1;      /* Shared Memory Indicator */
int gnShmID2;      /* Shared Memory Indicator */
int gnSemID1;      /* Semapore Indicator */
int gnSemID2;      /* Semapore Indicator */
pthread_t threads[MAX_PLAYERS];
char buffer[BUFFER_SIZE];

/* Shared Memory */
key_t keyShm;       /* Shared Memory Key */
_ST_SHM *pstShm2;      /* 공용 메모리 구조체 */

/* Semapore */
key_t keySem;       /* Semapore Key */
struct sembuf mysem_open  = {0, -1, SEM_UNDO}; // 세마포어 얻기
struct sembuf mysem_close = {0, 1, SEM_UNDO};  // 세마포어 돌려주기

int my_sum;
int dealer_sum;

/*ctrl+C 로 종료시 공유자원 해제*/
void *set_shutdown ()
{
        printf("[SIGNAL] : Got shutdown signal\n");
        shmctl( gnShmID2, IPC_RMID, 0 );
        printf("[SIGNAL] : Shared meory segment marked for deletion\n");;
        semctl( gnSemID2, IPC_RMID, 0 );
        printf("[SIGNAL] : Semapore segment marked for deletion\n");
        exit (1);
}

/*데이터 전송 스레드*/
void* send_msg(void *x)
{
  //sleep(0.2);
  int choice;
  char strChoice[10];
  while(1){
    /* 게임 종료시그널을 받으면 전송 스레드도 종료*/
    if(pstShm2->finalcheck){break;}

    /*HIT STAND를 보내야 할 상황일 때*/
    if(pstShm2->check2){
      printf("\n");
      printf("1. Hit\n");
      printf("2. Stand\n");
      printf("Please choose 1 or 2: ");
      fflush(stdout);

      /*문자열 제한 get*/
      fgets(strChoice, sizeof(strChoice), stdin);
      choice = atoi(strChoice);
      /*HIT을 선택할 경우*/
      if (choice == 1)
      {
        /*임계영역*/
        if( semop(gnSemID2, &mysem_open, 1) == -1 )
        {
              perror("semop");
              exit(1);
        }
        strcpy(buffer, HIT);
        printf("Sending: %s\n", buffer);
        /* HIT send */
        strncpy(pstShm2->data, buffer, BUFFER_SIZE);
        pstShm2->check2 = 0;
        semop(gnSemID2, &mysem_close, 1);
        /*임계영역*/
        sleep(1);
      }
      /*STAND를 선택한 경우*/
      else if (choice == 2)
      {
        /*임계영역*/
        if( semop(gnSemID2, &mysem_open, 1) == -1 )
        {
              perror("semop");
              exit(1);
        }
        strcpy(buffer, STAND);
        printf("Sending: %s\n", buffer);
        strncpy(pstShm2->data, buffer, BUFFER_SIZE);
        pstShm2->check2 = 0;
        pstShm2->check = 0;
        semop(gnSemID2, &mysem_close, 1);
        /*임계영역*/
        break;
      }
      else
        printf("Unrecognized choice. Choose again.\n");
      /*문자열 초기화*/
      strChoice[strlen(strChoice)-1] = '\0';
    }
  }
}


/*데이터 수신 스레드*/
void* recv_msg(void* x)
{
  /*게임 플레이에 쓸 카드 변수들 */
  int my_hand_values[20], dealer_hand_values[20];
  int my_hand_suits[20], dealer_hand_suits[20];
  int nmy = 0, ndealer = 0;
  /* server side - send first cardset */
  printf("게임 시작 대기중...\n");


  while(1) {
    /*첫 카드덱을 받을 경우*/
     if(pstShm2->check==1){
       /*임계영역*/
       if( semop(gnSemID2, &mysem_open, 1) == -1 )
       {
               perror("semop");
               exit(1);
       }
       strncpy(buffer, pstShm2->data, BUFFER_SIZE);
       printf("received : %s\n", buffer);


       /*내 패에 받은 카드를 저장*/
       my_hand_values[0] = get_value_id(buffer[0]);
       my_hand_suits[0] = get_suit_id(buffer[1]);
       my_hand_values[1] = get_value_id(buffer[2]);
       my_hand_suits[1] = get_suit_id(buffer[3]);
       dealer_hand_values[0] = get_value_id(buffer[4]);
       dealer_hand_suits[0] = get_suit_id(buffer[5]);
       nmy = 2;
       ndealer = 1;

       int choice;
       /*합산*/
       my_sum = calc_sum(my_hand_values, nmy);

       printf("\n");
       /*카드덱과 합산값 화면출력*/
       printf("My Hand: ");
       display_state(my_hand_values, my_hand_suits, nmy);
       printf("Dealer Hand: ");
       display_state(dealer_hand_values, dealer_hand_suits, ndealer);
       /*체크 시그널 수정*/
       pstShm2->check = 0;
       pstShm2->check2 = 1;

       semop(gnSemID2, &mysem_close, 1);
       /*임계영역*/
     }

     /* 내 덱에 합계가 21이 넘어갈 경우 lose */
     if (my_sum > 21)
     {
       printf("\nI'm busted! I lose!\n");
       /*shared memory detached*/
       if(shmdt(pstShm2) == -1) {
          perror("shmdt failed");
          exit(1);
       }
       break;
     }

     /*HIT하여 새로운 카드 받는 부분*/
     if(pstShm2->check==2){
       /******* 임계영역 *******/
       if( semop(gnSemID2, &mysem_open, 1) == -1 )
       {
               perror("semop");
               exit(1);
       }
       /* recv new card */
       strncpy(buffer, pstShm2->data, BUFFER_SIZE);

       printf("I received: %s\n", buffer);
       /*내 패에 받은 카드를 저장*/
       my_hand_values[nmy] = get_value_id(buffer[0]);
       my_hand_suits[nmy] = get_suit_id(buffer[1]);
       ++nmy;
       printf("\n");
       printf("My Hand: ");
       /*클라이언트의 카드 합산이 21이 넘어갈 경우 게임 끝 시그널 설정*/
       pstShm2->finalcheck = display_state(my_hand_values, my_hand_suits, nmy);
       printf("Dealer Hand: ");
       /*카드덱과 합산값 화면출력*/
       display_state(dealer_hand_values, dealer_hand_suits, ndealer);
       /*체크 시그널 수정*/
       pstShm2->check = 0;
       pstShm2->check2 = 1;
       semop(gnSemID2, &mysem_close, 1);
       /******* 임계영역 *******/
     }
     /*게임 끝 시그널일 경우*/
     if(pstShm2->finalcheck){
       pstShm2->finalcheck=0;
       /******* 임계영역 *******/
       if( semop(gnSemID2, &mysem_open, 1) == -1 )
       {
               perror("semop");
               exit(1);
       }
       unsigned i;
       /* 딜러의 패를 받아온다. */
       strncpy(buffer, pstShm2->data, BUFFER_SIZE);
       pstShm2->check = 0;
       pstShm2->check2 = 0;
       printf("I received: %s\n", buffer);
       /*딜러의 패 합산*/
       for (i = 0; i < strlen(buffer); i += 2)
   		{
   			dealer_hand_values[ndealer] = get_value_id(buffer[i]);
   			dealer_hand_suits[ndealer] = get_suit_id(buffer[i + 1]);
   			++ndealer;
   		}
       semop(gnSemID2, &mysem_close, 1);
       /******* 임계영역 *******/

       printf("\n");
       /*마지막 패 보여주기*/
   		printf("My Hand: ");
   		pstShm2->finalcheck = display_state(my_hand_values, my_hand_suits, nmy);
   		printf("Dealer Hand: ");
   		display_state(dealer_hand_values, dealer_hand_suits, ndealer);

     	my_sum = calc_sum(my_hand_values, nmy);
     	dealer_sum = calc_sum(dealer_hand_values, ndealer);
      /*게임 승패 정보 출력*/
     	if (dealer_sum > 21)
     		printf("\nDealer busted! I win!\n");
     	else if (my_sum == dealer_sum)
     		printf("\nMe and the dealer have the same score. It's a push!\n");
     	else if (my_sum < dealer_sum)
     		printf("\nDealer has a higher score. I lose!\n");
     	else
     		printf("\nI have a higher score. I win!\n");
      break;
     }

  }
  /*shared memory detached*/
  if(shmdt(pstShm2) == -1) {
     perror("shmdt failed");
     exit(1);
  }
}

/*게임 시작 스레드 새로운 공유메모리와 세마포어 설정*/
void* play_game(void * id)
{

  int shmId;
  int semId;
  /*공유메모리와 세마포어의 키값을 클라이언트 ID에 맞춰 생성*/
  shmId = (intptr_t)id + 60100;
  semId = (intptr_t)id+1 +60100;
  keyShm = (key_t)shmId;
  /* 공유 메모리 세그먼트를 연다 - 필요하면 만든다. */
  if( (gnShmID2 = shmget( keyShm, SEGSIZE, IPC_CREAT | IPC_EXCL | 0666 )) == -1 )
  {
          printf("Shared memory segment exist - opening as client\n");
          /* 세그먼트가 이미 존재한다면 - try as a client */
          if( (gnShmID2 = shmget( keyShm, SEGSIZE, 0 )) == -1 )
          {
                  perror("shmget");
                  exit(1);
          }
  }
  else
  {
          printf("Creating new shared memory segment\n");
  }

  /* 현재 프로세스에 공유 메모리 세그먼트를 연결한다. */
  if( (pstShm2 = (_ST_SHM *)shmat(gnShmID2, 0, 0)) == NULL )
  {
          perror("shmat");
          exit(1);
  }

  /* Semapore 키값을 생성한다. */
  keySem = (key_t)semId;

  /* 공유 Semapore 세그먼트를 연다 - 필요하면 만든다. */
  if( (gnSemID2 = semget( keySem, 1, IPC_CREAT | IPC_EXCL | 0666 )) == -1 )
  {
          printf("Semapore segment exist - opening as client\n");
          /* 세그먼트가 이미 존재한다면 - try as a client */
          if( (gnSemID2 = semget( keySem, 0, 0 )) == -1 )
          {
                  perror("semget");
                  exit(1);
          }
  }
  else
  {
          printf("Creating new semapore segment\n");
  }
  /*recv_msg, send_msg 스레드 생성*/
  pthread_create(&threads[1], NULL, recv_msg, NULL);
  pthread_create(&threads[2], NULL, send_msg, NULL);
  int tid;
  /*스레드들 끝날때까지 대기후 삭제*/
  for (tid = 1; tid < 2; ++tid)
    pthread_join(threads[tid], NULL);
}






void shmClntVersion()
{
  /* Shared Memory */
  key_t        keyShm;       /* Shared Memory Key */
  key_t        keySem;       /* Semapore Key */
  _ST_SHM        *pstShm1;      /* 공용 메모리 구조체 */
  char buffer[BUFFER_SIZE];
  /* Shared Memory의 키값을 생성한다. */
  keyShm = (key_t)60100;
  keySem = (key_t)60101;
  int id;
  struct sembuf mysem_open  = {0, -1, SEM_UNDO}; // 세마포어 얻기
  struct sembuf mysem_close = {0, 1, SEM_UNDO};  // 세마포어 돌려주기

  /* 공유 메모리 세그먼트를 연다 - 필요하면 만든다. */
  if( (gnShmID1 = shmget( keyShm, SEGSIZE, IPC_CREAT | IPC_EXCL | 0666 )) == -1 )
  {
          printf("Shared memory segment exist - opening as client\n");
          /* Segment probably already exists - try as a client */
          if( (gnShmID1 = shmget( keyShm, SEGSIZE, 0 )) == -1 )
          {
                  perror("shmget");
                  exit(1);
          }
  }
  else
  {
          printf("Creating new shared memory segment\n");
  }

  /* 현재 프로세스에 공유 메모리 세그먼트를 연결한다. */
  if( (pstShm1 = (_ST_SHM *)shmat(gnShmID1, 0, 0)) == NULL )
  {
          perror("shmat");
          exit(1);
  }

  /* 공유 Semapore 세그먼트를 연다 - 필요하면 만든다. */
  if( (gnSemID1 = semget( keySem, 1, IPC_CREAT | IPC_EXCL | 0666 )) == -1 )
  {
          printf("Semapore segment exist - opening as client\n");
          /* Segment probably already exists - try as a client */
          if( (gnSemID1 = semget( keySem, 0, 0 )) == -1 )
          {
                  perror("semget");
                  exit(1);
          }
  }
  else
  {
          printf("Creating new semapore segment\n");
  }

  /* Signal 등록 */
  (void) signal (SIGINT, (void (*)()) set_shutdown);


  while(1) {

    printf("input \"start\" ==> ");
    /*start를 입력하면 게임시작*/
    fgets(buffer, BUFFER_SIZE, stdin);

    /* 공유메모리에 데이터 쓰기 */
    strncpy(pstShm1->data, buffer, BUFFER_SIZE);
    pstShm1->check = 1;
    /* server가 먼저 실행 */
    while(pstShm1->check);

    /******* 임계영역 *******/
    if( semop(gnSemID1, &mysem_open, 1) == -1 )
    {
           perror("semop");
           exit(1);
    }
    /* 쓴 데이터가 ‘quit’이면 while 문 벗어남 */
    if(!strncmp(pstShm1->data, "quit", 4)) {
       break;
    }

    /*몇번째 클라이언트 ID인지를 받아온다.*/
    strcpy(buffer, pstShm1->data);
    printf("recv: %s\n", buffer);
    semop(gnSemID1, &mysem_close, 1);
    /******* 임계영역 *******/
    id=buffer[0]-48;
    /*play_game 스레드 진입*/
    pthread_create(&threads[0], NULL, play_game, (void*)(intptr_t) id);
    /*스레드 종료 대기후 삭제*/
    pthread_join(threads[0], NULL);
    printf("game end!\n");

    /* shared memory & semapore del */
    printf("[SIGNAL] : Shared meory segment marked for deletion\n");
    shmctl( gnShmID2, IPC_RMID, 0 );
    printf("[SIGNAL] : Semapore segment marked for deletion\n");
    semctl( gnSemID2, IPC_RMID, 0 );

    break;
  }
  if(shmdt(pstShm1) == -1) {
     perror("shmdt failed");
     exit(1);
  }
}
