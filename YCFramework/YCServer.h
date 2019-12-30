#pragma once

class ClientHandle
{
public:
	SOCKET			_sock;
	SOCKADDR_IN		_addr;
};
class IOData
{
public:
	OVERLAPPED		_overlap;
	WSABUF			_wsabuf;
	char			_buffer[1024];

	enum eIOTYPE
	{
		Read,
		Write
	};
	eIOTYPE _type;
};


// ----------------------------------------------------------------------------
//		## Server ##
// ----------------------------------------------------------------------------
// 기본적으로 상속받아서 사용하면 됨. 상속받은 뒤 세션과, 업데이트 로직을
// 커스텀 하여 사용한다.
// ----------------------------------------------------------------------------
class YCServer
{
private:
	int PORT;

	WSAData wsaData;
	HANDLE CP;
	SOCKET mServerSock;
	SOCKADDR_IN	sAddr;

	vector<thread> IOCP_Thread;
	vector<thread> workerThreads;

private:
	void Init();

public:
	class JobManager* mJobManager;
	class Strand* Job;

public:
	YCServer(int port) : PORT(port) { Init(); }
	~YCServer();

	void Srv_Start();
	void Update();

	bool Srv_Startup();
	bool Srv_bind();
	bool Srv_listen();
	void Srv_RunWorker();
	void Srv_Accept();
	void Srv_Disconnect(SOCKET);
};