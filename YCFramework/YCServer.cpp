#include "pch.h"

#include "YCServer.h"
#include "JobManager.h"
#include "YCPacket.h"



void YCServer::Init()
{
	printf(" ========== Server Start ==========\n");
	mJobManager = new JobManager();
	Job = new Strand(*mJobManager);
}

YCServer::~YCServer()
{
	delete mJobManager;
}

void YCServer::Srv_Start()
{
	Time::UpdateDeltaTime();
	if (!Srv_Startup()) 
	{
		printf("StartUP Erorr\n");
		return;
	}

	Srv_RunWorker();

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	for (unsigned int i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
	{
		IOCP_Thread.push_back(std::thread([this]()
		{
			SOCKET sock;
			DWORD bytesTrans;
			ClientHandle* CH;
			IOData* IO;

			while (true)
			{
				if (GetQueuedCompletionStatus(CP, &bytesTrans, (PULONG_PTR)&CH, (LPOVERLAPPED*)&IO, INFINITE))
				{
					sock = CH->_sock;

					switch (IO->_type)
					{
					case IO->Read:
					{
						if (bytesTrans == 0)
						{
							Srv_Disconnect(sock);
							delete IO;
							continue;
						}
						printf("메시지 받음\n");
						auto c(IO->clnts);

						char* buf = new char[bytesTrans];
						std::copy(IO->_buffer, IO->_buffer + bytesTrans, buf);
						
						c->Job->Add([c, buf, bytesTrans] { c->Read(buf, bytesTrans); });
						delete IO;
					}
					break;
					case IO->Write:
					{
						printf("[%d][byte:%d] 전송완료!\n", packetNumber++, IO->_wsabuf.len);
						delete IO;
					}
					break;
					}
				}
				else
				{
					sock = CH->_sock;
					if (bytesTrans == 0)
					{
						Srv_Disconnect(sock);
						delete IO;
					}
				}


			}
		}));
	}

	if (!Srv_bind())
	{
		printf("bind Erorr\n");
		return;
	}
	if (!Srv_listen())
	{
		printf("listen Erorr\n");
		return;
	}

	// -------------------------------------------------------------------------
	//		 ## Server Server Jobs
	// -------------------------------------------------------------------------
	// 이곳에서 서버가 해야할 일을 동기적으로 수행합니다.
	// -------------------------------------------------------------------------
	Time::UpdateDeltaTime();
	Job->Add([this] { Update(); });
	// -------------------------------------------------------------------------


	Srv_Accept();

	for (auto& i : IOCP_Thread)		i.join();
	for (auto& i : workerThreads)	i.join();

	closesocket(mServerSock);

	WSACleanup();
}



void YCServer::Update()
{
	Time::UpdateDeltaTime();

	static float dt = 0;
	static size_t FPS = 0;

	dt += Time::deltaTime;
	FPS++;
	if (dt > 1)
	{
		dt = 0;
		printf("Server FPS : %lld\n", FPS);
		FPS = 0;
	}

	Job->Add([this] { Update(); });
}

bool YCServer::Srv_Startup()
{
	bool StartUp_error = WSAStartup(MAKEWORD(2, 2), &wsaData) == -1;

	CP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	return !StartUp_error;
}

bool YCServer::Srv_bind()
{
	mServerSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	bool SocketCreate_error = mServerSock == INVALID_SOCKET;

	memset(&sAddr, 0, sizeof(sAddr));
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	sAddr.sin_port = htons(PORT);

	bool Bind_error = bind(mServerSock, (SOCKADDR*)&sAddr, sizeof(sAddr)) == -1;

	return !(SocketCreate_error || Bind_error);
}

bool YCServer::Srv_listen()
{
	return !listen(mServerSock, 5);
}

void YCServer::Srv_RunWorker()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	for (unsigned int i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
	{
		workerThreads.push_back(std::move(thread([this] { mJobManager->run(); })));
	}
}

void YCServer::Srv_Accept()
{
	while (true)
	{
		SOCKET clientSocket;
		SOCKADDR_IN	cAddr;
		int addrLen = sizeof(cAddr);

		clientSocket = accept(mServerSock, (SOCKADDR*)&cAddr, &addrLen);

		#pragma region Client accept Error
		bool accept_error =
			clientSocket == INVALID_SOCKET;

		if (!accept_error)	printf("client connect!\n");
		else				printf("accept error!\n");
		#pragma endregion

		auto ch = (ClientHandle*)malloc(sizeof(ClientHandle));
		ch->_sock = clientSocket;
		memcpy(&(ch->_addr), &cAddr, addrLen);

		CreateIoCompletionPort((HANDLE)clientSocket, CP, (ULONG_PTR)ch, 0);

		Job->Add([clientSocket, this, ch]
		{
			auto client = Create(clientSocket);

			client->CH = ch;

			printf("client connected [ ID : %lld ]\n", client->mSock);
			client->Job->Add([clientSocket, client]
			{
				int recvByte = 0, flag = 0;

				auto clientIO = new IOData();
				memset(&clientIO->_overlap, 0, sizeof(OVERLAPPED));
				clientIO->_type = clientIO->Read;
				clientIO->_wsabuf.buf = clientIO->_buffer;
				clientIO->_wsabuf.len = BUFSIZ;
				clientIO->clnts = client;
				WSARecv(clientSocket, &(clientIO->_wsabuf), 1, (LPDWORD)&recvByte, (LPDWORD)&flag, &(clientIO->_overlap), NULL); 
			});
		});
	}
}

void YCServer::Srv_Disconnect(SOCKET sock)
{
	Job->Add([sock]
	{

	});
}