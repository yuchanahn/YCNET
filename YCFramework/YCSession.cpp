#include "pch.h"
#include "YCSession.h"

void YCSession::Init()
{
}

YCSession::~YCSession()
{
	printf("Safe Client Disconnect! [ ID : %lld ]\n", mSocket);
	closesocket(mSocket);
	free(CH);
	if (Job) delete Job;
}

void YCSession::Write(char* data, int len)
{
	auto IO = new IOData;
	memset(&IO->_overlap, 0, sizeof(OVERLAPPED));
	IO->_type = IO->Write;
	std::copy(data, data + len, IO->_buffer);
	IO->_wsabuf.buf = IO->_buffer;
	IO->_wsabuf.len = len;
	IO->clnts = Self;
	IO->_wsabuf.buf[len] = '\0';
	printf("전송할 데이터 [size : %d]\n", IO->_wsabuf.len);

	WSASend(mSocket, &IO->_wsabuf, 1, NULL, 0, &IO->_overlap, NULL);
}

void YCSession::Read(char* data, int len)
{
	auto IO = new IOData;
	memset(&IO->_overlap, 0, sizeof(OVERLAPPED));
	IO->_wsabuf.len = 1024;
	IO->_wsabuf.buf = IO->_buffer;
	IO->_type = IO->Read;
	IO->clnts = Self;

	int flag = 0;
	WSARecv(mSocket, &IO->_wsabuf, 1, NULL, (LPDWORD)&flag, &IO->_overlap, NULL);
}