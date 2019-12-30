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
// �⺻������ ��ӹ޾Ƽ� ����ϸ� ��. ��ӹ��� �� ���ǰ�, ������Ʈ ������
// Ŀ���� �Ͽ� ����Ѵ�.
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