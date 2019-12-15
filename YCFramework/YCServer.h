#pragma once

class ClientHandle
{
public:
	SOCKET			_sock;
	SOCKADDR_IN		_addr;
};

class ClientData : public std::enable_shared_from_this<ClientData>
{
	
public:
	static map<SOCKET, shared_ptr<ClientData>> Clients;
	class Strand* mStrand = nullptr;
	ClientHandle* CH;

protected:
	



	// =====================================================================
	//		## Write
	//		����ȭ ����;
	//		���ο��� ���.
	// =====================================================================
	void AsyncWrite(char* data, int len);
public:
	ClientData(class Strand*, SOCKET sock);
	~ClientData();

	bool IsOnLine;
	
	// =====================================================================
	//		## Read
	//		����ȭ ����;
	//		�� Ŭ���� ���ο����� ����.
	// =====================================================================
	void AsyncRead(char* data, int len);

	void SetOffLine();

	SOCKET mSock;
};


class ClientIO
{
public:
	OVERLAPPED		_overlap;
	WSABUF			_wsabuf;
	char			_buffer[1024];
	shared_ptr<ClientData> clnts;

	enum eIOTYPE
	{
		Read,
		Write
	};
	eIOTYPE _type;
};



// ===========================================================================
//		## TODO :
//	Sesstion ����� �μǿ��� �����ϱ�.
//  �ڵ� ����ȭ �ϴ� Async Read �� Async Wirte �����.
//  ��Ŷ �����, ��� ���� ���� �����, ��Ŷ ���Ŀ��°� ó��.
//  ����Ƽ���� ��ǥ ����ȭ -> ����? �̷��� ��ġ �ְ� �ű�� ���� �ϱ�.
//	�α��� ó�� �����, �� �����, ä�� �����.
//	���� ����� ���� ����.
//	���� ��������� ����.
//	������ ���� ����.
//	��ų �����.
//	��í �����.
// ===========================================================================


// ----------------------------------------------------------------------------
//		## Server
// ----------------------------------------------------------------------------

class YCServer
{
	int PORT = -1;

	void Init();


	WSAData wsaData;

	HANDLE CP;


	SOCKET				mServerSock;
	SOCKADDR_IN			sAddr;

	vector<std::thread*> wThread;
	vector<thread> workerThreads;

	atomic<int> packetNumber = 0;
	SYSTEM_INFO sysInfo;
protected:
	class JobManager* jobManager;

public:
	YCServer(int port) : PORT(port) { Init(); }
	~YCServer();
	void Srv_Start();


	bool Srv_Startup();
	bool Srv_bind();
	bool Srv_listen();

	void Srv_RunWorker();

	void Srv_Accept();

	void Srv_Disconnect(SOCKET);

	static class Strand* SSrd;

	shared_ptr<ClientData> Create(SOCKET cSock);
};