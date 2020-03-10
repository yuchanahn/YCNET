#include "pch.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")

#include "YCClient.h"
#include "YCPacket.h"
#include <winsock2.h>



YCClient::YCClient()
{
	r = new YC_Packet_ReadManager();
}

YCClient::~YCClient()
{
	closesocket(socket);
	WSACleanup();
}

bool YCClient::connect(const char* ip_address, u_short port)
{
	if (mIp_address != ip_address)
	{
		mIp_address = (char*)ip_address;

		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return -1;
		socket = ::socket(AF_INET, SOCK_STREAM, 0);

		if (socket == INVALID_SOCKET) return false;

		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr(ip_address);
		serveraddr.sin_port = htons(port);
	}

	if (::connect(socket, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

int YCClient::read_packet()
{
	auto received = recv(socket, buf, 1024, 0);
	if (received == SOCKET_ERROR)
		return SOCKET_ERROR;
	else if (received == 0)
		return 0;
	
	r->read((unsigned char*)buf, received);

	return 1;
}

