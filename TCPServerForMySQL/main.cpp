#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include "jdbc/mysql_connection.h"
#include "jdbc/cppconn/driver.h"
#include "jdbc/cppconn/exception.h"
#include "jdbc/cppconn/resultset.h"
#include "jdbc/cppconn/statement.h"
#include "jdbc/cppconn/prepared_statement.h"

#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#pragma comment(lib, "debug/mysqlcppconn.lib")
#else
#pragma comment(lib, "mysqlcppconn.lib")
#endif 

using namespace std;

std::string Utf8ToMultiByte(std::string utf8_str);

int main()
{
	bool bRunning = true;
	
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN ServerAddrIn;
	memset(&ServerAddrIn, 0, sizeof(SOCKADDR_IN));
	ServerAddrIn.sin_family = AF_INET;
	ServerAddrIn.sin_port = htons(1234);
	ServerAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY;

	fd_set Original;
	fd_set CopyReads;
	timeval Timeout;
	Timeout.tv_sec = 0;
	Timeout.tv_usec = 10;

	FD_ZERO(&Original);
	FD_SET(ServerSocket, &Original);

	bind(ServerSocket, (SOCKADDR*)&ServerAddrIn, sizeof(SOCKADDR_IN));

	listen(ServerSocket, 0);
	printf("Server\n");

	while (bRunning)
	{
		CopyReads = Original;

		//polling
		int fd_num = select(0, &CopyReads, 0, 0, &Timeout);

		if (fd_num == 0)
		{
			continue;
		}
		if (fd_num == SOCKET_ERROR)
		{
			bRunning = false;
			break;
		}

		for (size_t i = 0; i < Original.fd_count; ++i)
		{
			//����� ���� ����Ʈ �߿� �̺�Ʈ �߻� ����
			if (FD_ISSET(Original.fd_array[i], &CopyReads))
			{
				if (Original.fd_array[i] == ServerSocket)
				{
					//���� ���Ͽ� �̺�Ʈ�� �߻���, ���� ��û
					SOCKADDR_IN ClientSockAddrIn;
					memset((char*)&ClientSockAddrIn, 0, sizeof(SOCKADDR_IN));
					int ClientSockAddrInSize = sizeof(SOCKADDR_IN);
					SOCKET ClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientSockAddrIn, &ClientSockAddrInSize);
					FD_SET(ClientSocket, &Original);
					std::cout << "Connected client : " << ClientSocket << std::endl;
				}
				else
				{
					SOCKET ClientSocket = Original.fd_array[i];
					//Client ����, Recv, Send
					char Buffer[1024] = { 0, };

					//blocking, non blocking
					//packet ����, protocol 
					int RecvLength = recv(ClientSocket, Buffer, sizeof(Buffer), 0);

					if (RecvLength == 0)
					{
						//���� ����
						std::cout << "Disconnected Client." << std::endl;
						FD_CLR(ClientSocket, &Original);
						closesocket(ClientSocket);
					}
					else if (RecvLength < 0)
					{
						//Error
						std::cout << "Disconnected Client By Error : " << GetLastError() << std::endl;
						FD_CLR(ClientSocket, &Original);
						closesocket(ClientSocket);
					}
					else
					{
						//�ڷᰡ ������ ó��
						//Packet Parse
						int Number = 0;
						int WinCount = 0;
						int TotalGamePlay = 0;

						memcpy(&Number, &Buffer[0], 4);
						memcpy(&WinCount, &Buffer[4], 4);
						memcpy(&TotalGamePlay, &Buffer[8], 4);

						try
						{
							sql::Driver* driver; //workbench
							sql::Connection* connection; //����
							sql::Statement* statement; //query
							sql::ResultSet* resultset; //���ȭ��
							sql::PreparedStatement* preparedStatement;

							//work��ġ �ѱ�
							driver = get_driver_instance();

							//���� ��ư ������
							connection = driver->connect("tcp://127.0.0.1:3306", "root", "12dnjf3dlfwjsdur@");

							if (connection == nullptr)
							{
								cout << "connect failed" << endl;
								exit(-1);
							}

							//��� �����ͺ��̽� ����(use)
							connection->setSchema("logindata");

							preparedStatement = connection->prepareStatement("insert into new_table(number, wincount, totalgameplay) values( ?, ? ,?)");
							preparedStatement->setInt(1, Number);
							preparedStatement->setInt(2, WinCount);
							preparedStatement->setInt(3, TotalGamePlay);
							preparedStatement->executeUpdate();

							statement = connection->createStatement();
							resultset = statement->executeQuery("select * from new_table ");

							for (;resultset->next();)
							{
								cout << resultset->getInt("number") << " : " <<
									Utf8ToMultiByte(resultset->getString("wincount")) << " : " <<
									Utf8ToMultiByte(resultset->getString("totalgameplay")) << endl;
							}

							delete resultset;
							delete preparedStatement;
							delete connection;
						}
						catch (sql::SQLException e)
						{
							cout << "# ERR: SQLException in " << __FILE__;
							cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
							cout << "# ERR: " << e.what();
							cout << " (MySQL error code: " << e.getErrorCode();
							cout << ", SQLState: " << e.getSQLState() << " )" << endl;
						}


						/*for (size_t i = 0; i < Original.fd_count; ++i)
						{
							if (Original.fd_array[i] != ServerSocket)
							{
								std::cout << Original.fd_array[i] << std::endl;
								int SentLength = send(Original.fd_array[i], Buffer, 8, 0);
							}
						}*/

					}
				}
			}
		}
	}

	closesocket(ServerSocket);

	WSACleanup();
	return 0;
}

std::string Utf8ToMultiByte(std::string utf8_str)
{
	std::string resultString; char* pszIn = new char[utf8_str.length() + 1];
	strncpy_s(pszIn, utf8_str.length() + 1, utf8_str.c_str(), utf8_str.length());
	int nLenOfUni = 0, nLenOfANSI = 0; wchar_t* uni_wchar = NULL;
	char* pszOut = NULL;
	// 1. utf8 Length
	if ((nLenOfUni = MultiByteToWideChar(CP_UTF8, 0, pszIn, (int)strlen(pszIn), NULL, 0)) <= 0)
		return nullptr;
	uni_wchar = new wchar_t[nLenOfUni + 1];
	memset(uni_wchar, 0x00, sizeof(wchar_t) * (nLenOfUni + 1));
	// 2. utf8 --> unicode
	nLenOfUni = MultiByteToWideChar(CP_UTF8, 0, pszIn, (int)strlen(pszIn), uni_wchar, nLenOfUni);
	// 3. ANSI(multibyte) Length
	if ((nLenOfANSI = WideCharToMultiByte(CP_ACP, 0, uni_wchar, nLenOfUni, NULL, 0, NULL, NULL)) <= 0)
	{
		delete[] uni_wchar; return 0;
	}
	pszOut = new char[nLenOfANSI + 1];
	memset(pszOut, 0x00, sizeof(char) * (nLenOfANSI + 1));
	// 4. unicode --> ANSI(multibyte)
	nLenOfANSI = WideCharToMultiByte(CP_ACP, 0, uni_wchar, nLenOfUni, pszOut, nLenOfANSI, NULL, NULL);
	pszOut[nLenOfANSI] = 0;
	resultString = pszOut;
	delete[] uni_wchar;
	delete[] pszOut;
	return resultString;
}


