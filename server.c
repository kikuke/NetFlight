#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#define BUF_SIZE 100 //메시지를 받을 곳 크기 100
#define MAX_CLNT 2 //최대 수용 플레이어
#define SERVER_PORT 2388 //게임 서버의 PORT지정

unsigned WINAPI HandleClnt(void* arg); //각 클라이언트의 처리 스레드 함수
void SendMsg(char* msg, int len); //메시지 전송 함수
void ErrorHandling(char* msg); //에러처리 함수

int clntCnt = 0; //현재 클라이언트 카운트
SOCKET clntSocks[MAX_CLNT]; //최대 클라이언트 수 만큼 소켓 파일 디스크립터 저장 배열
HANDLE hMutex; //동기화를 위한 뮤텍스

int main()
{
	WSADATA wsaData; //윈속 초기화 정보 전달받을 자료구조
	SOCKET hServSock, hClntSock; //운영체제에서 할당해 준 소켓 파일 디스크립터를 받을 곳 각 서버, 클라이언트임.
	SOCKADDR_IN servAdr, clntAdr; //생성할 서버에 관련된 데이터를 담을곳/수신받은 클라이언트 정보에 관한 데이터를 담을 곳
	int clntAdrSz;//클라이언트 정보 사이즈
	HANDLE hThred;//스레드 핸들 정보 저장용

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)//윈속 2.2버전으로 초기화후 그 정보 wsaData에 저장
		ErrorHandling("WSAStartup() error!");//에러메시지 출력

	hMutex = CreateMutex(NULL, FALSE, NULL);//동기화를 위한 뮤텍스 생성
	hServSock = socket(PF_INET, SOCK_STREAM, 0);//운영체제에 IPV4 프로토콜로, 스트림소켓 유형중 0번(TCP)소켓을 할당해달라고 요청한 후 파일 디스크립터를 받아옴

	memset(&servAdr, 0, sizeof(servAdr));//서버용 소켓 생성 데이터를 저장할 자료구조 전체 비트를 0으로 세팅
	servAdr.sin_family = AF_INET;//인터넷 프로토콜 주소체계로 값 할당. 인터넷 프로토콜을 이용하겠다.
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);//자신에게 온 어떤 NIC의 IP든 허용
	servAdr.sin_port = htons(SERVER_PORT);//리스너 포트 번호를 네트워크 전송을 위해 빅엔디안으로 변환.

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) //해당 소켓에 위에서 servAdr의 정보(ip, 포트 등)를 입력
		ErrorHandling("bind() error");
	if (listen(hServSock, 2) == SOCKET_ERROR) //최대 서버 대기인원 2명
		ErrorHandling("listen() error");

	puts("20192388 김진수");
	while (TRUE)
	{
		if (clntCnt < MAX_CLNT)//현재 클라이언트가 최대인원이 아닐경우 연결허용
		{
			clntAdrSz = sizeof(clntAdr);//accept에 주소크기를 저장한 데이터를 넘길 변수
			hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSz);//해당 소켓에 접속한 클라이언트의 연결을 허용, 새로운 소켓 부여

			WaitForSingleObject(hMutex, INFINITE);//소켓 목록 동기화를 위한 뮤텍스 대기
			clntSocks[clntCnt++] = hClntSock;//새로 생성된 소켓 등록
			ReleaseMutex(hMutex);//뮤텍스 이용 종료

			hThred = (HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&hClntSock, 0, NULL);//새로 만들어진 소켓을 인자로 넘기며 해당 클라이언트의 처리 스레드 시작
			printf("Connected client IP: %s\n", inet_ntoa(clntAdr.sin_addr));//클라이언트 ip 표시
		}
	}
	closesocket(hServSock);//소켓 종료
	WSACleanup();//
	return 0;//윈속 초기화
}

unsigned WINAPI HandleClnt(void* arg)
{
	SOCKET hClntSock = *((SOCKET*)arg);//보이드포인터로 받은 소켓 파일디스크립터 복구
	int strLen = 0, i;//수신데이터 크기 저장용, 루프용
	char msg[BUF_SIZE];//각 클라이언트별 수신/전송버퍼

	WaitForSingleObject(hMutex, INFINITE);//플레이어 넘버 동기화를 위한 뮤텍스 대기
	msg[0] = clntCnt; msg[1] = '\0';//현재 클라이언트 수를 이용한 플레이어를 전송할 메시지 첫번째 비트에 추가. 그 다음 비트는 메시지 종료를 의미
	send(clntSocks[clntCnt - 1], msg, 2, 0);//접속한 대상에게 부여할 플레이어 넘버를 전송.
	ReleaseMutex(hMutex);//뮤텍스 이용 종료

	if (clntCnt == MAX_CLNT)//2명 모두 왔을경우 스타트
	{
		SendMsg("Ready 3\n", sizeof("Ready 3\n"));//대기 안내 메시지
		Sleep(1000);
		SendMsg("Ready 2\n", sizeof("Ready 3\n"));//대기 안내 메시지
		Sleep(1000);
		SendMsg("Ready 1\n", sizeof("Ready 3\n"));//대기 안내 메시지
		Sleep(1000);
		SendMsg("Start", sizeof("Start"));//Start메시지 전송. 이 메시지를 수신한 클라이언트들은 게임이 시작됨을 알게되어 관련 처리를 하게됨.
	}
	while ((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0)//이후 클라이언트들이 전송하는 메시지들을 에코로 방송해줌
		SendMsg(msg, strLen);

	WaitForSingleObject(hMutex, INFINITE);//소켓 목록 동기화를 위한 뮤텍스 대기
	for (i = 0; i < clntCnt; i++)//접속을 종료한 클라이언트 소켓 목록에서 제거 처리
	{
		if (hClntSock == clntSocks[i])
		{
			while (i++ < clntCnt - 1)
				clntSocks[i] = clntSocks[i + 1];
			break;
		}
	}
	clntCnt--;
	ReleaseMutex(hMutex);//뮤텍스 이용 종료
	closesocket(hClntSock);//소켓 종료
	return 0;
}

void SendMsg(char* msg, int len)//메시지 전송함수
{
	int i;
	WaitForSingleObject(hMutex, INFINITE);//메시지 전송 동기화를 위한 뮤텍스 대기
	for (i = 0; i < clntCnt; i++)
		send(clntSocks[i], msg, len, 0);//등록된 소켓들 모두에게 설정한 메시지 전달
	ReleaseMutex(hMutex);//뮤텍스 이용 종료
}

void ErrorHandling(char* msg)//에러 표시용
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}