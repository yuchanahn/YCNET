#pragma once

class ClientHandle
{
public:
	SOCKET			_sock;
	SOCKADDR_IN		_addr;
};


class ClientIO
{
public:
	OVERLAPPED		_overlap;
	WSABUF			_wsabuf;
	char			_buffer[1024];
	class ClientData* clnts;

	enum eIOTYPE
	{
		Read,
		Write
	};
	eIOTYPE _type;
};



class ClientData
{
public:
	SOCKET sock;
	int id;
};

