#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#define BUF_SIZE 100 //�޽����� ���� �� ũ�� 100
#define SERVER_IP "127.0.0.1" //������ ���� ������ IP����
#define SERVER_PORT 2388 //������ ���� ������ PORT����
#define MAP_SIZE 20 //���� ���� ũ��

unsigned WINAPI SendMsg(void* arg); //���� Ŭ���̾�Ʈ �޽��� ���� ������ �Լ�
unsigned WINAPI RecvMsg(void* arg); //���� �޽��� ���� ������ �Լ�
void ErrorHandling(char* msg); //���� üũ�Լ�
void DrawGame(); //���� ȭ�� �׸��� �Լ�
void PlayerMove(char* buf); //���� ���� �����͸� �̿��� �÷��̾� ��ġ�� �������ִ� �Լ�

char msg[BUF_SIZE]; //���Ӹ޽��� ���۹���
char map[MAP_SIZE][MAP_SIZE]; //�� ǥ�� �迭
int isGameStart=0; //0�̸� �����غ���, 1�̸� ���ӽ���
int my_Playnum; //�������� �ο����� �÷��̾� ��ȣ
int player[2][2]; //p1, p2�� �ʻ� ��ġ

int main(void)
{
	WSADATA wsaData; //���� �ʱ�ȭ ���� ���޹��� �ڷᱸ��
	SOCKET hSock; //�ü������ �Ҵ��� �� ���� ���� ��ũ���͸� ���� ��
	SOCKADDR_IN servAdr; //������ ������ ���õ� ������
	HANDLE hSndThread, hRcvThread; //�ü������ �Ҵ��� �� �ڵ� ��ȣ ���� ��

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)//���� 2.2�������� �ʱ�ȭ�� �� ���� wsaData�� ����
		ErrorHandling("WSAStartup() error!");//�����޽��� ���

	hSock = socket(PF_INET, SOCK_STREAM, 0);//�ü���� IPV4 �������ݷ�, ��Ʈ������ ������ 0��(TCP)������ �Ҵ��ش޶�� ��û�� �� ���� ��ũ���͸� �޾ƿ�

	memset(&servAdr, 0, sizeof(servAdr));//���� ���� �����͸� �޾ƿ� �ڷᱸ�� ��ü ��Ʈ�� 0���� ����
	servAdr.sin_family = AF_INET;//���ͳ� �������� �ּ�ü��� �� �Ҵ�. ���ͳ� ���������� �̿��ϰڴ�.
	servAdr.sin_addr.s_addr = inet_addr(SERVER_IP);//���ڿ��� �ּ��������� ��ȯ. ip�� ����
	servAdr.sin_port = htons(SERVER_PORT);//��Ʈ ��ȣ�� ��Ʈ��ũ ������ ���� �򿣵������ ��ȯ.

	if(connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)//hSock������ �̿��� servAdr���������� ����. �̶� �Լ� ��Ŀ� ���� (SOCKADDR*)�� ����ȯ. ������ �����޽��� ���
		ErrorHandling("connect() error!");

	puts("20192388 ������");
	hSndThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);//SendMsg�����带 ������ �ڵ� ���� �޾ƿ���. ���� ���ڷ� �̿����� ���� ����
	hRcvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSock, 0, NULL);//RecvMsg�����带 ������ �ڵ� ���� �޾ƿ���. ���� ���ڷ� �̿����� ���� ����

	WaitForSingleObject(hSndThread, INFINITE);//�����尡 ���� ������ ������ ���
	WaitForSingleObject(hRcvThread, INFINITE);//�����尡 ���� ������ ������ ���
	closesocket(hSock);//���� �ݱ� ��û
	WSACleanup();//���� �ʱ�ȭ
	return 0;
}

unsigned WINAPI SendMsg(void* arg)//�޽��� �����Լ�
{
	SOCKET hSock = *((SOCKET*)arg);//���̵� ������ ���ڷ� �޾Ҵ� ���� �ٽ� SOCKET���� �ǵ�����
	while (TRUE)
	{
		msg[0] = _getch(); //���ھ��� Ű���� �� �ϳ� �޾Ƽ� ������ ������ �޽��� ù��° ��Ʈ�� �Ҵ�
		msg[1] = my_Playnum; //������ ������ �޽��� �ι�° ��Ʈ�� ���� Ŭ���̾�Ʈ�� �÷��̾�ѹ� ���� �ֱ�
		msg[2] = '\n'; //�޽��� �������� ǥ��

		send(hSock, msg, 3, 0);//hSock�� �̿��� 3��Ʈ �޽��� ����
		Sleep(300);//���� �Է�ó�� ������ ���� ����
	}
	return 0;
}

unsigned WINAPI RecvMsg(void* arg)//�޽��� �����Լ�
{
	int hSock = *((SOCKET*)arg);//���̵� ������ ���ڷ� �޾Ҵ� ���� �ٽ� SOCKET���� �ǵ�����
	char buf[BUF_SIZE];//�۽Ź��ۿ� ������ ���� ������ ���� �������� ����
	int strLen;//�޽��� ���� ����
	while (TRUE)
	{
		strLen = recv(hSock, buf, sizeof(buf)-1, 0); //�޽��� ���� �� �޽��� ���� ����. 
		if (strLen == -1) //������ ����
			return -1;

		if (buf[0] == 1 || buf[0] == 2)//���� �����Ϳ��� ���޹��� ù��° ��Ʈ ���� 1�Ǵ� 2�ϰ��
		{
			my_Playnum = buf[0];//���޹��� �ѹ��� �÷��̾� �ѹ� �ʱ�ȭ
			printf("Your PlayerNum: %d\n", my_Playnum);
		}

		if (strstr(buf, "Start") != NULL)//���� ���޹��� ���ڿ��� Start�� �������� ��� ���� ����. tcp�� ��Ʈ�� �����̱⿡ �����Ͱ� �������� ���� ��Ŷ������ ��Ŷ�� ���ļ� �� �۰� ������ �ֱ⿡ strstr����.
		{
			Sleep(1000);//���� ������ �����ֱ� ���� ��� ����

			isGameStart = 1;//���� ��ŸƮ �÷��� set
			player[0][0] = 0 + 1;//�÷��̾�1�� x �ʱ� ��ġ
			player[0][1] = MAP_SIZE / 2 + 1;//�÷��̾�1�� y �ʱ� ��ġ

			player[1][0] = MAP_SIZE - 2;//�÷��̾�2�� x �ʱ� ��ġ
			player[1][1] = MAP_SIZE / 2 - 1;//�÷��̾�2�� y �ʱ� ��ġ

			for (int i = 0; i < MAP_SIZE; i++)//���� �� ������ ����
				for (int j = 0; j < MAP_SIZE; j++)
				{
					if (i == 0 || i == MAP_SIZE - 1 || j == 0 || j == MAP_SIZE - 1)
						map[i][j] = '#';
					else
						map[i][j] = ' ';
				}

			map[player[0][1]][player[0][0]] = '@';//�ʱ� �÷��̾� ��ġ ǥ��
			map[player[1][1]][player[1][0]] = '@';//�ʱ� �÷��̾� ��ġ ǥ��
			DrawGame();//������ ���� �׷���
		}

		if (isGameStart)//���� ���� �÷��װ� �����Ǿ��������
		{
			PlayerMove(buf);//���� ���� ������ �÷��̾ �������� ������
			DrawGame();//������ ����� ���� �ٽ� �׸�
		}

		buf[strLen] = 0;//���ڿ� �� ǥ��

		//���߿� �ּ�ó���ϱ�
		fputs(buf, stdout);//���� ���ڿ�(����) ���
	}
	return 0;
}

void PlayerMove(char* buf)//���۰��� ���� �÷��̾ ������
{
	if (buf[0] == 'w')//���� �����̱�
	{
		if (player[buf[1] - 1][1] - 1 > 0)//���۰��� �ι�° ��Ʈ�� � �÷��̾ �޽����� ���´��� ������ �ִµ� �̸� �̿��� �÷��̾ ������. �̶� �ش� �÷��̾ ���� ����ԵǴ��� �˻�.
		{
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = ' ';//������ �÷��̾��� ���� ��ġ�� �������� �ٲ���
			player[buf[1] - 1][1] -= 1;//�÷��̾��� ��ġ�� ���� �̵�
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = '@';//������ �÷��̾��� ��ġ�� �ʿ� �ٽ� �׷���
		}
	}
	else if (buf[0] == 's')//�Ʒ��� �����̱� w�϶��� �۵� ����� ����.
	{
		if (player[buf[1] - 1][1] + 1 < MAP_SIZE - 1)
		{
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = ' ';
			player[buf[1] - 1][1] += 1;
			map[player[buf[1] - 1][1]][player[buf[1] - 1][0]] = '@';
		}
	}
	else if (buf[0] == ' ')//�Ѿ� �߻��ϱ� � �÷��̾ �߻��ߴ��Ŀ� ���� ó���� ���ݾ� �ٸ�
	{
		if (buf[1] == 1)//1�� �÷��̾ �߻����� ���
		{
			for (int i = player[buf[1] - 1][0] + 1; i < MAP_SIZE - 1; i++)//1�� �÷��̾��� �ٷ� ������ ���� ������ �� ������ �˻� �� ó��
			{
				if (map[player[buf[1] - 1][1]][i] == '@')//��� �÷��̾ �ش� ������ ���� ���
				{
					map[player[buf[1] - 1][1]][i] = '!';//��� �÷��̾��� ǥ�ø� !�� �ٲ�
					DrawGame();//���� �ٽ� �׸�
					Sleep(2000);//2�ʴ��

					system("cls");//ȭ�� �����
					puts("20192388 ������");
					puts("Player1 Win!");//�÷��̾�1 �¸�â
					Sleep(10000);
				}

				map[player[buf[1] - 1][1]][i] = '*';//�ش� ������ ���� *�� ǥ��
			}
		}
		else if(buf[1] == 2)//2�� �÷��̾ �߻����� ���
		{
			for (int i = player[buf[1] - 1][0] - 1; i > 0; i--)//2�� �÷��̾��� �ٷ� ���� ���� ���� �� ������ �˻� �� ó��
			{//�۵� ����� �÷��̾�1�� ����
				if (map[player[buf[1] - 1][1]][i] == '@')
				{
					map[player[buf[1] - 1][1]][i] = '!';
					DrawGame();
					Sleep(2000);

					system("cls");
					puts("20192388 ������");
					puts("Player2 Win!");
					Sleep(10000);
				}

				map[player[buf[1] - 1][1]][i] = '*';
			}
		}
	}
}

void DrawGame()//�� �׸��� �Լ�
{
	system("cls");//ȭ�� �ʱ�ȭ
	puts("20192388 ������");
	puts("Move: W, S; Shot: Space;");//���� ����Ű ǥ��
	for (int i = 0; i < MAP_SIZE; i++)//�� ��ü�� ���� �ش� ������ ȭ�鿡 ǥ��
	{
		for (int j = 0; j < MAP_SIZE; j++)
			putc(map[i][j], stdout);
		putc('\n', stdout);
	}
	for (int i = player[0][0] + 1; i < MAP_SIZE - 1; i++)//���� 1�� �÷��̾� ������ �Ѿ� �߻�� ������ �־��ٸ� �����
		if(map[player[0][1]][i] == '*')
			map[player[0][1]][i] = ' ';
	for (int i = player[1][0] - 1; i > 0; i--)//���� 2�� �÷��̾� ������ �Ѿ� �߻�� ������ �־��ٸ� �����
		if (map[player[0][1]][i] == '*')
			map[player[1][1]][i] = ' ';
}

void ErrorHandling(char* msg)//���� ǥ�� �Լ�
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}