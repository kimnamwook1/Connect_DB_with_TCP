#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")


int main()
{
	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN ServerAddrIn;
	memset(&ServerAddrIn, 0, sizeof(SOCKADDR_IN));
	ServerAddrIn.sin_family = PF_INET;
	ServerAddrIn.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerAddrIn.sin_port = htons(1234);

	printf("client\n");

	int Numnber = 0;
	int WinCount = 0;
	int TotalGamePlay = 0;

	printf("Please Type Number: ");
	scanf("%d", &Numnber);
	printf("Please Type WinCount: ");
	scanf("%d", &WinCount);
	printf("Please Type TotalGamePlay: ");
	scanf("%d", &TotalGamePlay);

	char Buffer[1024] = { 0, };

	memcpy(Buffer, &Numnber, 4);
	memcpy(&Buffer[4], &WinCount, 4);
	memcpy(&Buffer[8], &TotalGamePlay, 4);

	connect(ServerSocket, (SOCKADDR*)&ServerAddrIn, sizeof(SOCKADDR_IN));

	int SentLength = send(ServerSocket, Buffer, 12, 0);
	printf("Sent Length : %d\n", SentLength);

	if (SentLength == 0)
	{
		//연결 종료
		std::cout << "Disconnected Client." << std::endl;
	}
	else if (SentLength < 0)
	{
		//Error
		std::cout << "Disconnected Client By Error : " << GetLastError() << std::endl;
	}

	WSACleanup();

	//return 0;
}