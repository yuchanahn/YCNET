#pragma once


struct Base;

class PackData
{
	int size;
public:
	char* data;

	template <typename T>
	auto Get() {
		return ((T*)m_GetBase(data))->UnPack();
	}
	template <typename T, typename T2>
	void Get(T2* target) {
		((T*)m_GetBase(data))->UnPackTo(target);
	}




	Base* m_GetBase(char* data_);

	PackData(char* data_, int len) {
		size = len;
		data = new char[len];
		std::copy(data_, data_+len, data);
	};
	~PackData() {
		delete[] data;
	};

private:
};


class ClientHandle
{
public:
	SOCKET			_sock;
	SOCKADDR_IN		_addr;
};

class ClientData : public std::enable_shared_from_this<ClientData>
{
	char* mBuffer = nullptr;
	int mBuf_len = 0;
	
public:
	class UserBase* User = nullptr;
	static map<SOCKET, shared_ptr<ClientData>> Clients;
	class Strand* mStrand = nullptr;
	ClientHandle* CH;

protected:
	




	// =====================================================================
	//		## Write
	//		동기화 안함;
	//		내부에서 사용.
	// =====================================================================
	void AsyncWrite(char* data, int len);
public:
	ClientData(class Strand*, SOCKET sock);
	~ClientData();

	void StrandAsyncWrite(char* data, int len);

	void StrandAsyncWriteAll(char* data, int len);
	void Boomed();

	bool IsOnLine;
	
	// =====================================================================
	//		## Read
	//		동기화 안함;
	//		이 클래스 내부에서만 실행.
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
//	Sesstion 만들고 셰션에서 관리하기.
//  자동 동기화 하는 Async Read 랑 Async Wirte 만들기.
//  패킷 만들기, 헤더 테일 구조 만들기, 패킷 뭉쳐오는것 처리.
//  유니티에서 좌표 동기화 -> 보간? 미래의 위치 주고 거기로 가게 하기.
//	로그인 처리 만들기, 핑 만들기, 채팅 만들기.
//	몬스터 만들고 스텟 적용.
//	몬스터 드랍아이템 적용.
//	아이템 장착 적용.
//	스킬 만들기.
//	가챠 만들기.
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


	map<int, struct fmobT*> mobs;
	

	atomic<int> packetNumber = 0;
	SYSTEM_INFO sysInfo;
protected:
	class JobManager* jobManager;

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

	static class Strand* SSrd;

	shared_ptr<ClientData> Create(SOCKET cSock);
};