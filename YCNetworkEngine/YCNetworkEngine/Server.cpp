#include "pch.h"
//#include "Time.h"
//
//
//class ClientHandle
//{
//public:
//	SOCKET			_sock;
//	SOCKADDR_IN		_addr;
//};
//
//
//
//class ClientIO
//{
//public:
//	OVERLAPPED		_overlap;
//	WSABUF			_wsabuf;
//	char			_buffer[1024];
//	class Session*	mSession;
//
//	enum eIOTYPE
//	{
//		Read,
//		Write
//	};
//	eIOTYPE _type;
//
//	ClientIO(class Session* session, eIOTYPE type, char* buffer, int len) : mSession(session), _type(type)
//	{
//		memset(&_overlap, 0, sizeof(OVERLAPPED));
//		_wsabuf.buf = _buffer;
//
//		if(buffer != nullptr)
//			for (int i = 0; i < len; i++) _buffer[i] = buffer[i];
//		_wsabuf.len = len;
//	}
//};
//
//
//CRITICAL_SECTION g_cs;
//
//
//queue<function <void()>> g_JabList;
//
//class Session
//{
//	int mSock;
//	static map<int, Session*> Sessions;
//public:
//	Session(int sock) : mSock(sock) {}
//	static Session* Create(int sock)
//	{
//		return Sessions[sock] = new Session(sock);
//	}
//	void PostRecv(char* data, int len)
//	{
//		for(auto i : Session::Sessions) i.second->PostSend(data, len);
//
//		ClientIO* IO = new ClientIO(this, IO->Read, nullptr, len);
//
//		int flag = 0;
//		WSARecv(mSock, &IO->_wsabuf, 1, NULL, (LPDWORD)&flag, &IO->_overlap, NULL);
//	}
//	void PostSend(char* data, int len)
//	{
//		ClientIO* IO = new ClientIO(this, IO->Write, data, len);
//		WSASend(mSock, &IO->_wsabuf, 1, NULL, 0, &IO->_overlap, NULL);
//	}
//};
//map<int, Session*> Session::Sessions;
//
//
//void post(function<void()> jab)
//{
//	Lock(&g_cs) { g_JabList.push(jab); }
//}
//
//void ServerStart()
//{
//
//	static int FPS = 0;
//	static float t = 0;
//	FPS++;
//	if (t >= 1)
//	{
//		std::cout << "FPS: " << FPS << std::endl;
//		FPS = 0;
//		t = 0;
//	}
//
//	post( ServerStart );
//}
//
//
//
//int main()
//{
//	InitializeCriticalSection(&g_cs);
//
//	WSAData wsaData;
//
//	HANDLE CP;
//
//	ClientIO* clientIO;
//	ClientHandle* clientHandle;
//
//	SOCKET				mServerSock;
//	SOCKADDR_IN			sAddr;
//
//	vector<std::thread*> wThread;
//
//	bool StartUp_error =
//		WSAStartup(wVersion, &wsaData) == -1;
//
//
//	CP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
//
//	SYSTEM_INFO sysInfo;
//	GetSystemInfo(&sysInfo);
//
//	//thread 생성. 스레드에 CP전달. 람다로 하면 될듯.
//	for (int i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
//	{
//		wThread.push_back(new std::thread([CP]()
//		{
//			SOCKET sock;
//			DWORD bytesTrans;
//			ClientHandle*	CH;
//			ClientIO*		IO;
//
//			while (true)
//			{
//				if (!GetQueuedCompletionStatus(CP, &bytesTrans, (PULONG_PTR)&CH, (LPOVERLAPPED*)&IO, 0) && IO == NULL)
//				{
//					function<void()> jab;
//					Lock(&g_cs) 
//					{
//						if (!g_JabList.empty()) 
//						{
//							jab = g_JabList.front();
//							g_JabList.pop();
//						}
//					}
//					if(jab) jab();
//					continue;
//				}
//
//
//				/*
//				if (!GetQueuedCompletionStatus(CP, &bytesTrans, (PULONG_PTR)& CH, (LPOVERLAPPED*)& IO, INFINITE))
//				{
//					//Warker Thread 일시키기....
//					return;
//				}
//				*/
//
//				sock = CH->_sock;
//
//				switch (IO->_type)
//				{
//				case IO->Read:
//				{
//					printf("메시지 받음\n");
//
//					if (bytesTrans == 0) { closesocket(sock); free(CH); free(IO); continue; }
//					IO->mSession->PostRecv(IO->_buffer, bytesTrans);
//					free(IO);
//
//					//WSASend 에서 IO메모리 접근 후 데이터를 사용하기 때문에 메모리 재 할당해준다....
//
//					//IO = (ClientIO*)malloc(sizeof(ClientIO));
//					//memset(&IO->_overlap, 0, sizeof(OVERLAPPED));
//					//IO->_wsabuf.len = 1024;
//					//IO->_wsabuf.buf = IO->_buffer;
//					//IO->_type = IO->Read;
//
//					//int flag = 0;
//					//WSARecv(sock, &IO->_wsabuf, 1, NULL, (LPDWORD)&flag, &IO->_overlap, NULL);
//				}
//				break;
//				case IO->Write:
//				{
//					printf("message sent!\n");
//					free(IO);
//				}
//				break;
//				}
//			}
//		}));
//	}
//
//
//	mServerSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
//
//	bool SocketCreate_error =
//		mServerSock == INVALID_SOCKET;
//
//
//
//	memset(&sAddr, 0, sizeof(sAddr));
//	sAddr.sin_family = AF_INET;
//	sAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
//	sAddr.sin_port = htons(1234);
//
//
//	bool Bind_error =
//		bind(mServerSock, (SOCKADDR*)&sAddr, sizeof(sAddr)) == -1;
//
//	bool listen_error =
//		listen(mServerSock, 5);
//
//	post( ServerStart );
//
//	while (true)
//	{
//		SOCKET			cSock;
//		SOCKADDR_IN		cAddr;
//		int addrLen = sizeof(cAddr);
//
//		cSock = accept(mServerSock, (SOCKADDR*)&cAddr, &addrLen);
//		bool accept_error =
//			cSock == INVALID_SOCKET;
//
//		if (!accept_error)	printf("client connect!\n");
//		else				printf("accept error!\n");
//
//		clientHandle = (ClientHandle*)malloc(sizeof(ClientHandle));
//		clientHandle->_sock = cSock;
//		memcpy(&(clientHandle->_addr), &cAddr, addrLen);
//
//		CreateIoCompletionPort((HANDLE)cSock, CP, (ULONG_PTR)clientHandle, 0);
//		
//		clientIO = new ClientIO(Session::Create(cSock), clientIO->Read, nullptr, 1024);
//
//		int recvByte = 0, flag = 0;
//
//		WSARecv(cSock, &(clientIO->_wsabuf), 1, (LPDWORD)&recvByte, (LPDWORD)&flag, &(clientIO->_overlap), NULL);
//	}
//
//	for (auto i : wThread) i->join();
//
//	closesocket(mServerSock);
//
//	WSACleanup();
//}