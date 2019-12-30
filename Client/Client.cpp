#include "pch.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <Ws2tcpip.h> 
#include <mutex>
#include <map>
#include <list>
#include <tuple>
#include "YCPacket.h"


#define BUFSIZE 1024


void err_display(const char* msg);
void err_quit(const char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);

int main()
{
	SOCKET sock;
	SOCKADDR_IN serveraddr;

	int retval;
	char buf[BUFSIZE + 1];
	int len;
	WSADATA wsa;


	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("sock()");

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serveraddr.sin_port = htons(51234);

	while (true)
	{
		retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) { Sleep(100); continue; }// err_quit("connect()");

		break;
	}

	std::thread th([&] {
		char buf2[BUFSIZE + 1];
		while (1) 
		{
			auto received = recv(sock, buf2, BUFSIZE, 0);
			if (received == SOCKET_ERROR)
				return SOCKET_ERROR;
			else if (received == 0)
				break;

			buf2[received] = '\0';
			printf("data:%s  len%d \n", buf2, received);
		}
		});

	while (1)
	{
		int c;
		ZeroMemory(buf, sizeof(buf));
		printf("send To server : ");

		int packet_number = 0;
		int delayT = 0;

		printf("패킷 갯수 : ");
		std::cin >> packet_number;
		printf("딜레이 타임 : ");
		std::cin >> delayT;
		printf("패킷 종류 (1:Thransfrom, 2:Player, 3:Ping)\n");
		std::cin >> c;
		/*
		HeaderTail ht;
		if (c == 1)
		{
			pTransfromData Tr{};
			Tr.data.x = -3;
			Tr.data.y = 3;

			char* buf2;
			int len2;

			YCPacketBuilder::pack<packTransfromData, pTransfromData>(Tr, buf2, len2);

			ht = get_header_tail(buf2, len2);
		}
		else if(c == 2)
		{
			pPlayerData playerTh{ };
			playerTh.data.Hp = 100;
			playerTh.data.Mp = 10;

			char* buf;
			int len;

			YCPacketBuilder::pack<packPlayerData, pPlayerData>(playerTh, buf, len);
			ht = get_header_tail(buf, len);
		}
		else if(c == 3)
		{
			pPing mping { };
			mping.data.time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

			char* buf;
			int len;

			YCPacketBuilder::pack<packPing, pPing>(mping, buf, len);
			ht = get_header_tail(buf, len);
		}

		
		len = ht.len;
		
		for (int i = 0; i < packet_number; i++)
		{
			send(sock, ht.data, len, 0);
			Sleep(delayT);
		}
		*/
	}


	th.join();

	closesocket(sock);
	WSACleanup();

	return 0;
}

int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;


	while (left > 0)
	{
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}
	return (len - left);
}

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}
