The Blackjack Game Using IPCs
=============================

FIFO, MEssage Queue, Shared Memory를 이용하여 만든 blackjack 게임입니다.

Division
===============
[박인효](https://github.com/PARKINHYO?tab=repositories) : Message Queue <br>
[김진엽](https://github.com/Jinyeob) : FIFO <br>
[이진재](https://github.com/loftmain) : Shared Memory


How to install
==================

저장소를 클론하고 컴파일합니다.

    $ git clone https://github.com/PARKINHYO/BlackjackGame.git
    $ cd BlackjackGame
    $ make

코드는 각 IPC 디렉토리에 있습니다. 컴파일시 각 IPC 디렉토리에 빌드된 파일이 만들어집니다. 서버와 클라이언트 실행시에 게임이 시작됩니다.

example : Message Queue

    terminal1
    $ ./server

    terminal2
    $ ./client
    input data ==> start

Rules
========
* 에이스(A)는 자신에게 유리하게 1점 혹은 11점으로 계산하고 J, Q, K는 10점으로 계산합니다.
* 카드의 합이 21점 혹은 21점에 가장 가까운 사람이 승리합니다.
* 처음 2장의 카드가 21점이면 blackjack이됩니다.
* 플레이어는 blackjack이 아닌 경우 21점에 가까워지게 추가 카드를 요구할 수 있습니다.
* 플레이어는 판단에 따라 Hit(카드를 추가로 가져오거나) 혹은 Stand(더 이상 카드를 받지 않음)할 수 있습니다.
* 카드의 합이 21점을 초과해버릴 경우 0점으로 게임에서 지게 됩니다(Bust).


Specfication
===============

Header
---------

> variables

* ### const char* suits[]
    카드의 모양 ♠, ♥, ♣, ◆
* ### const char* values[]
    카드의 번호 A, 0~9, T, J, Q, K
* ### struct MSG {
    ### long type; 
    ### char data[BUFFER_SIZE]
    ### }
    메세지 큐 구조체


> functions
* ### void error(const char * msg)

* ### int get_suit_id(char suit)
    카드 모양의 index값을 추출
* ### int get_vlaue_id(char value)
    카드 번호의 index값을 추출
* ### int calc_sum(const int hand_values[], int ncards)
    현재 가진 카드값의 합을 리턴
* ### void display_state(int hand_values[], int hand_suits[], int ncards)
    현재까지 진행된 게임의 상태를 출력


Server 
----------

> variables

* ### int msgid_Send_Main, msgid_Recv_Main
    msgget()의 리턴값을 저장하는 변수
* ### pthread_mutex_t card_mutex
    mutex 변수
* ### ncard, card_values[52], card_suits[52]
    카드 개수, 카드번호, 모양
* ### players_hand_values[MAX_PLAYERS][20], dealers_hand_values[MAX_PLAYERS][20]
    플레이어, 딜러의 카드 번호
* ### players_hand_suits[MAX_PLAYERS][20], dealers_hand_suits[MAX_PLAYERS][20]
    플레이어, 딜러의 카드 모양


> functions


* ### void* play_game_one(void* data)
    플레이어와 같은 IPC key값을 가지고 IPC를 연결하여 blackjack 게임에서 dealer역할을 합니다. 게임이 시작되면 클라이언트에게 게이머가 사용하는 2장의 카드와 딜러가 가진 카드 한장을 보내줍니다. 이후에 클라이언트의 HIT, STAND 신호를 받아 카드를 더 주거나 게임을 끝내고 패를 보내줍니다.
* ### void* init_cards()
    52장의 카드를 랩덤하게 부여합니다.
* ### void init_cards()
    CTRL + c를 누를시 처리되는 시그널 처리 함수입니다.

Client 
-----------

> variables

* ### int msgid_Send_Main, msgid_Recv_Main

* ### pid_t pid
    클라이언트 프로세스 아이디 번호 저장
* ### int my_sum, dealer_sum
    플레이어와 딜러의 합산 값
* ### int check, check2, finalcheck
    제어를 위한 변수
* ### msg buf
    메세지큐 구조체

> functions

* ### void* play_game()
    IPC를 통해 "start" 문자열을 서버에 보내게 되면 서버에서 게임이 시작됩니다. recv_msg 스레드와 send_msg 스레드를 생성합니다.
* ### void* set_shutdown()
* ### void* send_msg()
    서버에게 데이터를 보냅니다(HIT, STAND).
* ### void* recv_msg()
    서버가 보내는 데이터를 받습니다. 초기 카드를 받아 출력하고, 이후 HIT, STAND를 보내 받아온 카드들을 출력하고 합산 결과, 게임 결과를 출력합니다.

License
================
MIT [License](https://github.com/PARKINHYO/BlackjackGame/blob/master/LICENSE) Copyright (c) 2019 INHYO PARK










