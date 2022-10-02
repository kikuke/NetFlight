#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#define BUF_SIZE 100
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2388
#define MAP_SIZE 20

unsigned WINAPI SendMsg(void* arg);
unsigned WINAPI RecvMsg(void* arg);
void ErrorHandling(char* msg);
void DrawGame();
void PlayerMove(char* buf);

char msg[BUF_SIZE];
char map[MAP_SIZE][MAP_SIZE];
int isGameStart=0;
int my_Playnum;
int player[2][2];//위치

int main(void)
{
	WSADATA wsaData;
	SOCKET hSock;
	SOCKADDR_IN servAdr;
	HANDLE hSndThread, hRcvThread;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hSock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = inet_addr(SERVER_IP);
	servAdr.sin_port = htons(SERVER_PORT);

	if(connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("connect() error!");

	hSndThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);
	hRcvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSock, 0, NULL);

	WaitForSingleObject(hSndThread, INFINITE);
	WaitForSingleObject(hRcvThread, INFINITE);
	closesocket(hSock);
	WSACleanup();
	return 0;
}

unsigned WINAPI SendMsg(void* arg)
{
	SOCKET hSock = *((SOCKET*)arg);
	while (TRUE)
	{
		msg[0] = _getch(); //입력없이
		msg[1] = my_Playnum;
		msg[2] = '\n';

		send(hSock, msg, 3, 0);
		Sleep(300);
	}
	return 0;
}

unsigned WINAPI RecvMsg(void* arg)
{
	int hSock = *((SOCKET*)arg);
	char buf[BUF_SIZE];
	int strLen;
	while (TRUE)
	{
		strLen = recv(hSock, buf, sizeof(buf)-1, 0);
		if (strLen == -1)
			return -1;

		if (buf[0] == 1 || buf[0] == 2)//플레이어 넘버 초기화
		{
			my_Playnum = buf[0];
			printf("Your PlayerNum: %d\n", my_Playnum);
		}

		if (strstr(buf, "Start") != NULL)
		{
			Sleep(1000);

			isGameStart = 1;
			player[0][0] = 0 + 1;//플레이어1의 x 초기 위치
			player[0][1] = MAP_SIZE / 2 + 1;//플레이어1의 y 초기 위치

			player[1][0] = MAP_SIZE - 2;//플레이어2의 x 초기 위치
			player[1][1] = MAP_SIZE / 2 - 1;//플레이어2의 y 초기 위치

			for (int i = 0; i < MAP_SIZE; i++)
				for (int j = 0; j < MAP_SIZE; j++)
				{
					if (i == 0 || i == MAP_SIZE - 1 || j == 0 || j == MAP_SIZE - 1)
						map[i][j] = '#';
					else
						map[i][j] = ' ';
				}

			map[player[0][1]][player[0][0]] = '@';
			map[player[1][1]][player[1][0]] = '@';
			DrawGame();
		}

		if (isGameStart)
		{
			PlayerMove(buf);
			DrawGame();
		}

		buf[strLen] = 0;

		//나중에 주석처리하기
		fputs(buf, stdout);
	}
	return 0;
}

void PlayerMove(char* buf)
{
	if (buf[0] == 'w')//위로 움직이기
	{
		if (player[buf[1] - 1][1] - 1 > 0)
		{
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = ' ';
			player[buf[1] - 1][1] -= 1;
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = '@';
		}
	}
	else if (buf[0] == 's')
	{
		if (player[buf[1] - 1][1] + 1 < MAP_SIZE - 1)
		{
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = ' ';
			player[buf[1] - 1][1] += 1;
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = '@';
		}
	}
	else if (buf[0] == ' ')
	{
		if (buf[1] == 1)
		{
			for (int i = player[buf[1] - 1][0] + 1; i < MAP_SIZE - 1; i++)
			{
				if (map[player[buf[1] - 1][1]][i] == '@')
				{
					map[player[buf[1] - 1][1]][i] = '!';
					DrawGame();
					Sleep(2000);

					system("cls");
					puts("Player1 Win!");
					Sleep(10000);
				}

				map[player[buf[1] - 1][1]][i] = '*';
			}
		}
		else if(buf[1] == 2)
		{
			for (int i = player[buf[1] - 1][0] - 1; i > 0; i--)
			{
				if (map[player[buf[1] - 1][1]][i] == '@')
				{
					system("cls");
					puts("Player2 Win!");
					Sleep(10000);
				}

				map[player[buf[1] - 1][1]][i] = '*';
			}
		}
	}
}

void DrawGame()
{
	system("cls");

	puts("Move: W, S; Shot: Space;");
	for (int i = 0; i < MAP_SIZE; i++)
	{
		for (int j = 0; j < MAP_SIZE; j++)
			putc(map[i][j], stdout);
		putc('\n', stdout);
	}
	for (int i = player[0][0] + 1; i < MAP_SIZE - 1; i++)
		if(map[player[0][1]][i] == '*')
			map[player[0][1]][i] = ' ';
	for (int i = player[1][0] - 1; i > 0; i--)
		if (map[player[0][1]][i] == '*')
			map[player[1][1]][i] = ' ';
}

void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}