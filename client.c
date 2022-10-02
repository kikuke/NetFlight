#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#define BUF_SIZE 100 //메시지를 받을 곳 크기 100
#define SERVER_IP "127.0.0.1" //연결할 게임 서버의 IP지정
#define SERVER_PORT 2388 //연결할 게임 서버의 PORT지정
#define MAP_SIZE 20 //게임 맵의 크기

unsigned WINAPI SendMsg(void* arg); //게임 클라이언트 메시지 전송 스레드 함수
unsigned WINAPI RecvMsg(void* arg); //서버 메시지 수신 스레드 함수
void ErrorHandling(char* msg); //에러 체크함수
void DrawGame(); //게임 화면 그리기 함수
void PlayerMove(char* buf); //서버 수신 데이터를 이용해 플레이어 위치를 움직여주는 함수

char msg[BUF_SIZE]; //게임메시지 전송버퍼
char map[MAP_SIZE][MAP_SIZE]; //맵 표현 배열
int isGameStart=0; //0이면 게임준비중, 1이면 게임시작
int my_Playnum; //서버에게 부여받을 플레이어 번호
int player[2][2]; //p1, p2의 맵상 위치

int main(void)
{
	WSADATA wsaData; //윈속 초기화 정보 전달받을 자료구조
	SOCKET hSock; //운영체제에서 할당해 준 소켓 파일 디스크립터를 받을 곳
	SOCKADDR_IN servAdr; //접속할 서버에 관련된 데이터
	HANDLE hSndThread, hRcvThread; //운영체제에서 할당해 준 핸들 번호 받을 곳

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)//윈속 2.2버전으로 초기화후 그 정보 wsaData에 저장
		ErrorHandling("WSAStartup() error!");//에러메시지 출력

	hSock = socket(PF_INET, SOCK_STREAM, 0);//운영체제에 IPV4 프로토콜로, 스트림소켓 유형중 0번(TCP)소켓을 할당해달라고 요청한 후 파일 디스크립터를 받아옴

	memset(&servAdr, 0, sizeof(servAdr));//서버 관련 데이터를 받아올 자료구조 전체 비트를 0으로 세팅
	servAdr.sin_family = AF_INET;//인터넷 프로토콜 주소체계로 값 할당. 인터넷 프로토콜을 이용하겠다.
	servAdr.sin_addr.s_addr = inet_addr(SERVER_IP);//문자열을 주소유형으로 변환. ip값 저장
	servAdr.sin_port = htons(SERVER_PORT);//포트 번호를 네트워크 전송을 위해 빅엔디안으로 변환.

	if(connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)//hSock소켓을 이용해 servAdr설정값으로 연결. 이때 함수 양식에 맞춰 (SOCKADDR*)로 형변환. 오류시 에러메시지 출력
		ErrorHandling("connect() error!");

	puts("20192388 김진수");
	hSndThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);//SendMsg스레드를 실행후 핸들 값을 받아오기. 전달 인자로 이용중인 소켓 전달
	hRcvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSock, 0, NULL);//RecvMsg스레드를 실행후 핸들 값을 받아오기. 전달 인자로 이용중인 소켓 전달

	WaitForSingleObject(hSndThread, INFINITE);//스레드가 끝날 때까지 무한히 대기
	WaitForSingleObject(hRcvThread, INFINITE);//스레드가 끝날 때까지 무한히 대기
	closesocket(hSock);//소켓 닫기 요청
	WSACleanup();//윈속 초기화
	return 0;
}

unsigned WINAPI SendMsg(void* arg)//메시지 전송함수
{
	SOCKET hSock = *((SOCKET*)arg);//보이드 포인터 인자로 받았던 소켓 다시 SOCKET으로 되돌리기
	while (TRUE)
	{
		msg[0] = _getch(); //에코없이 키보드 값 하나 받아서 서버로 전송할 메시지 첫번째 비트에 할당
		msg[1] = my_Playnum; //서버로 전송할 메시지 두번째 비트에 현재 클라이언트의 플레이어넘버 정보 넣기
		msg[2] = '\n'; //메시지 끝났음을 표현

		send(hSock, msg, 3, 0);//hSock을 이용해 3비트 메시지 전송
		Sleep(300);//과한 입력처리 방지를 위한 슬립
	}
	return 0;
}

unsigned WINAPI RecvMsg(void* arg)//메시지 수신함수
{
	int hSock = *((SOCKET*)arg);//보이드 포인터 인자로 받았던 소켓 다시 SOCKET으로 되돌리기
	char buf[BUF_SIZE];//송신버퍼와 데이터 꼬임 방지를 위한 수신전용 버퍼
	int strLen;//메시지 길이 저장
	while (TRUE)
	{
		strLen = recv(hSock, buf, sizeof(buf)-1, 0); //메시지 수신 후 메시지 길이 저장. 
		if (strLen == -1) //비정상 종료
			return -1;

		if (buf[0] == 1 || buf[0] == 2)//서버 데이터에서 전달받은 첫번째 비트 값이 1또는 2일경우
		{
			my_Playnum = buf[0];//전달받은 넘버로 플레이어 넘버 초기화
			printf("Your PlayerNum: %d\n", my_Playnum);
		}

		if (strstr(buf, "Start") != NULL)//만약 전달받은 문자열에 Start가 섞여있을 경우 게임 시작. tcp는 스트림 성격이기에 데이터가 서버에서 보낸 패킷수보다 패킷을 합쳐서 더 작게 보낼수 있기에 strstr로함.
		{
			Sleep(1000);//이전 정보를 보여주기 위해 잠시 슬립

			isGameStart = 1;//게임 스타트 플래그 set
			player[0][0] = 0 + 1;//플레이어1의 x 초기 위치
			player[0][1] = MAP_SIZE / 2 + 1;//플레이어1의 y 초기 위치

			player[1][0] = MAP_SIZE - 2;//플레이어2의 x 초기 위치
			player[1][1] = MAP_SIZE / 2 - 1;//플레이어2의 y 초기 위치

			for (int i = 0; i < MAP_SIZE; i++)//게임 맵 데이터 세팅
				for (int j = 0; j < MAP_SIZE; j++)
				{
					if (i == 0 || i == MAP_SIZE - 1 || j == 0 || j == MAP_SIZE - 1)
						map[i][j] = '#';
					else
						map[i][j] = ' ';
				}

			map[player[0][1]][player[0][0]] = '@';//초기 플레이어 위치 표시
			map[player[1][1]][player[1][0]] = '@';//초기 플레이어 위치 표시
			DrawGame();//설정된 맵을 그려줌
		}

		if (isGameStart)//게임 시작 플래그가 설정되어있을경우
		{
			PlayerMove(buf);//수신 값을 참고해 플레이어를 맵위에서 움직임
			DrawGame();//위에서 변경된 맵을 다시 그림
		}

		buf[strLen] = 0;//문자열 끝 표시

		//나중에 주석처리하기
		fputs(buf, stdout);//받은 문자열(버퍼) 출력
	}
	return 0;
}

void PlayerMove(char* buf)//버퍼값에 따라 플레이어를 움직임
{
	if (buf[0] == 'w')//위로 움직이기
	{
		if (player[buf[1] - 1][1] - 1 > 0)//버퍼값의 두번째 비트에 어떤 플레이어가 메시지를 보냈는지 정보가 있는데 이를 이용해 플레이어를 움직임. 이때 해당 플레이어가 맵을 벗어나게되는지 검사.
		{
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = ' ';//움직일 플레이어의 현재 위치를 공백으로 바꿔줌
			player[buf[1] - 1][1] -= 1;//플레이어의 위치를 위로 이동
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = '@';//움직인 플레이어의 위치를 맵에 다시 그려줌
		}
	}
	else if (buf[0] == 's')//아래로 움직이기 w일때와 작동 방식은 같음.
	{
		if (player[buf[1] - 1][1] + 1 < MAP_SIZE - 1)
		{
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = ' ';
			player[buf[1] - 1][1] += 1;
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = '@';
		}
	}
	else if (buf[0] == ' ')//총알 발사하기 어떤 플레이어가 발사했느냐에 따라 처리가 조금씩 다름
	{
		if (buf[1] == 1)//1번 플레이어가 발사했을 경우
		{
			for (int i = player[buf[1] - 1][0] + 1; i < MAP_SIZE - 1; i++)//1번 플레이어의 바로 오른쪽 부터 오른쪽 맵 끝까지 검사 및 처리
			{
				if (map[player[buf[1] - 1][1]][i] == '@')//상대 플레이어가 해당 범위에 있을 경우
				{
					map[player[buf[1] - 1][1]][i] = '!';//상대 플레이어의 표시를 !로 바꿈
					DrawGame();//맵을 다시 그림
					Sleep(2000);//2초대기

					system("cls");//화면 지우기
					puts("20192388 김진수");
					puts("Player1 Win!");//플레이어1 승리창
					Sleep(10000);
				}

				map[player[buf[1] - 1][1]][i] = '*';//해당 범위를 전부 *로 표시
			}
		}
		else if(buf[1] == 2)//2번 플레이어가 발사했을 경우
		{
			for (int i = player[buf[1] - 1][0] - 1; i > 0; i--)//2번 플레이어의 바로 왼쪽 부터 왼쪽 맵 끝까지 검사 및 처리
			{//작동 방식은 플레이어1과 동일
				if (map[player[buf[1] - 1][1]][i] == '@')
				{
					map[player[buf[1] - 1][1]][i] = '!';
					DrawGame();
					Sleep(2000);

					system("cls");
					puts("20192388 김진수");
					puts("Player2 Win!");
					Sleep(10000);
				}

				map[player[buf[1] - 1][1]][i] = '*';
			}
		}
	}
}

void DrawGame()//맵 그리기 함수
{
	system("cls");//화면 초기화
	puts("20192388 김진수");
	puts("Move: W, S; Shot: Space;");//게임 조작키 표시
	for (int i = 0; i < MAP_SIZE; i++)//맵 전체를 돌며 해당 정보를 화면에 표시
	{
		for (int j = 0; j < MAP_SIZE; j++)
			putc(map[i][j], stdout);
		putc('\n', stdout);
	}
	for (int i = player[0][0] + 1; i < MAP_SIZE - 1; i++)//만약 1번 플레이어 직선상에 총알 발사된 흔적이 있었다면 지우기
		if(map[player[0][1]][i] == '*')
			map[player[0][1]][i] = ' ';
	for (int i = player[1][0] - 1; i > 0; i--)//만약 2번 플레이어 직선상에 총알 발사된 흔적이 있었다면 지우기
		if (map[player[0][1]][i] == '*')
			map[player[1][1]][i] = ' ';
}

void ErrorHandling(char* msg)//에러 표시 함수
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}