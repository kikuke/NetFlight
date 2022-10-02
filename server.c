#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#define BUF_SIZE 100 //�޽����� ���� �� ũ�� 100
#define MAX_CLNT 2 //�ִ� ���� �÷��̾�
#define SERVER_PORT 2388 //���� ������ PORT����

unsigned WINAPI HandleClnt(void* arg); //�� Ŭ���̾�Ʈ�� ó�� ������ �Լ�
void SendMsg(char* msg, int len); //�޽��� ���� �Լ�
void ErrorHandling(char* msg); //����ó�� �Լ�

int clntCnt = 0; //���� Ŭ���̾�Ʈ ī��Ʈ
SOCKET clntSocks[MAX_CLNT]; //�ִ� Ŭ���̾�Ʈ �� ��ŭ ���� ���� ��ũ���� ���� �迭
HANDLE hMutex; //����ȭ�� ���� ���ؽ�

int main()
{
	WSADATA wsaData; //���� �ʱ�ȭ ���� ���޹��� �ڷᱸ��
	SOCKET hServSock, hClntSock; //�ü������ �Ҵ��� �� ���� ���� ��ũ���͸� ���� �� �� ����, Ŭ���̾�Ʈ��.
	SOCKADDR_IN servAdr, clntAdr; //������ ������ ���õ� �����͸� ������/���Ź��� Ŭ���̾�Ʈ ������ ���� �����͸� ���� ��
	int clntAdrSz;//Ŭ���̾�Ʈ ���� ������
	HANDLE hThred;//������ �ڵ� ���� �����

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)//���� 2.2�������� �ʱ�ȭ�� �� ���� wsaData�� ����
		ErrorHandling("WSAStartup() error!");//�����޽��� ���

	hMutex = CreateMutex(NULL, FALSE, NULL);//����ȭ�� ���� ���ؽ� ����
	hServSock = socket(PF_INET, SOCK_STREAM, 0);//�ü���� IPV4 �������ݷ�, ��Ʈ������ ������ 0��(TCP)������ �Ҵ��ش޶�� ��û�� �� ���� ��ũ���͸� �޾ƿ�

	memset(&servAdr, 0, sizeof(servAdr));//������ ���� ���� �����͸� ������ �ڷᱸ�� ��ü ��Ʈ�� 0���� ����
	servAdr.sin_family = AF_INET;//���ͳ� �������� �ּ�ü��� �� �Ҵ�. ���ͳ� ���������� �̿��ϰڴ�.
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);//�ڽſ��� �� � NIC�� IP�� ���
	servAdr.sin_port = htons(SERVER_PORT);//������ ��Ʈ ��ȣ�� ��Ʈ��ũ ������ ���� �򿣵������ ��ȯ.

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) //�ش� ���Ͽ� ������ servAdr�� ����(ip, ��Ʈ ��)�� �Է�
		ErrorHandling("bind() error");
	if (listen(hServSock, 2) == SOCKET_ERROR) //�ִ� ���� ����ο� 2��
		ErrorHandling("listen() error");

	puts("20192388 ������");
	while (TRUE)
	{
		if (clntCnt < MAX_CLNT)//���� Ŭ���̾�Ʈ�� �ִ��ο��� �ƴҰ�� �������
		{
			clntAdrSz = sizeof(clntAdr);//accept�� �ּ�ũ�⸦ ������ �����͸� �ѱ� ����
			hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSz);//�ش� ���Ͽ� ������ Ŭ���̾�Ʈ�� ������ ���, ���ο� ���� �ο�

			WaitForSingleObject(hMutex, INFINITE);//���� ��� ����ȭ�� ���� ���ؽ� ���
			clntSocks[clntCnt++] = hClntSock;//���� ������ ���� ���
			ReleaseMutex(hMutex);//���ؽ� �̿� ����

			hThred = (HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&hClntSock, 0, NULL);//���� ������� ������ ���ڷ� �ѱ�� �ش� Ŭ���̾�Ʈ�� ó�� ������ ����
			printf("Connected client IP: %s\n", inet_ntoa(clntAdr.sin_addr));//Ŭ���̾�Ʈ ip ǥ��
		}
	}
	closesocket(hServSock);//���� ����
	WSACleanup();//
	return 0;//���� �ʱ�ȭ
}

unsigned WINAPI HandleClnt(void* arg)
{
	SOCKET hClntSock = *((SOCKET*)arg);//���̵������ͷ� ���� ���� ���ϵ�ũ���� ����
	int strLen = 0, i;//���ŵ����� ũ�� �����, ������
	char msg[BUF_SIZE];//�� Ŭ���̾�Ʈ�� ����/���۹���

	WaitForSingleObject(hMutex, INFINITE);//�÷��̾� �ѹ� ����ȭ�� ���� ���ؽ� ���
	msg[0] = clntCnt; msg[1] = '\0';//���� Ŭ���̾�Ʈ ���� �̿��� �÷��̾ ������ �޽��� ù��° ��Ʈ�� �߰�. �� ���� ��Ʈ�� �޽��� ���Ḧ �ǹ�
	send(clntSocks[clntCnt - 1], msg, 2, 0);//������ ��󿡰� �ο��� �÷��̾� �ѹ��� ����.
	ReleaseMutex(hMutex);//���ؽ� �̿� ����

	if (clntCnt == MAX_CLNT)//2�� ��� ������� ��ŸƮ
	{
		SendMsg("Ready 3\n", sizeof("Ready 3\n"));//��� �ȳ� �޽���
		Sleep(1000);
		SendMsg("Ready 2\n", sizeof("Ready 3\n"));//��� �ȳ� �޽���
		Sleep(1000);
		SendMsg("Ready 1\n", sizeof("Ready 3\n"));//��� �ȳ� �޽���
		Sleep(1000);
		SendMsg("Start", sizeof("Start"));//Start�޽��� ����. �� �޽����� ������ Ŭ���̾�Ʈ���� ������ ���۵��� �˰ԵǾ� ���� ó���� �ϰԵ�.
	}
	while ((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0)//���� Ŭ���̾�Ʈ���� �����ϴ� �޽������� ���ڷ� �������
		SendMsg(msg, strLen);

	WaitForSingleObject(hMutex, INFINITE);//���� ��� ����ȭ�� ���� ���ؽ� ���
	for (i = 0; i < clntCnt; i++)//������ ������ Ŭ���̾�Ʈ ���� ��Ͽ��� ���� ó��
	{
		if (hClntSock == clntSocks[i])
		{
			while (i++ < clntCnt - 1)
				clntSocks[i] = clntSocks[i + 1];
			break;
		}
	}
	clntCnt--;
	ReleaseMutex(hMutex);//���ؽ� �̿� ����
	closesocket(hClntSock);//���� ����
	return 0;
}

void SendMsg(char* msg, int len)//�޽��� �����Լ�
{
	int i;
	WaitForSingleObject(hMutex, INFINITE);//�޽��� ���� ����ȭ�� ���� ���ؽ� ���
	for (i = 0; i < clntCnt; i++)
		send(clntSocks[i], msg, len, 0);//��ϵ� ���ϵ� ��ο��� ������ �޽��� ����
	ReleaseMutex(hMutex);//���ؽ� �̿� ����
}

void ErrorHandling(char* msg)//���� ǥ�ÿ�
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}