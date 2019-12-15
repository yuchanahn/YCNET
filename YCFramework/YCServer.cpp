#include "pch.h"

#include "YCServer.h"
#include "JobManager.h"


ClientData::ClientData(Strand* strand, SOCKET sock)
{
	mStrand = strand;
	this->mSock = sock;
	IsOnLine = true;
}

ClientData::~ClientData()
{
	// 소켓을 바로 닫지 않아서 이상한 일이 발생하는듯.
	printf("[ sock_id : %lld ]클라이언트 종료!\n", mSock);
	closesocket(mSock);
	free(CH);
	if (!mStrand) delete mStrand;
}

void ClientData::AsyncWrite(char* data, int len)
{
	if (!IsOnLine) return;

	auto Self(shared_from_this());
	auto IO = new ClientIO;
	memset(&IO->_overlap, 0, sizeof(OVERLAPPED));
	IO->_type = IO->Write;
	std::copy(data, data + len, IO->_buffer);
	IO->_wsabuf.buf = IO->_buffer;
	IO->_wsabuf.len = len;
	IO->clnts = Self;
	IO->_wsabuf.buf[len] = '\0';
	printf("전송할 데이터 : %s [size : %d]\n", IO->_wsabuf.buf, IO->_wsabuf.len);
	WSASend(mSock, &IO->_wsabuf, 1, NULL, 0, &IO->_overlap, NULL);
}

void ClientData::AsyncRead(char* data, int len)
{
	/// 패킷에 따른 처리...

	///

	shared_ptr<ClientData> Self(shared_from_this());
	YCServer::SSrd->push([data, len] 
	{
		for (auto& i : Clients)
		{
			auto _data = new char[len];
			std::copy(data, data + len, _data);
			
			i.second->mStrand->push([i, _data, len]
			{
				i.second->AsyncWrite(_data, len);
				delete[] _data;
			});
		}
		delete[] data;
	});

	auto IO = new ClientIO;
	memset(&IO->_overlap, 0, sizeof(OVERLAPPED));
	IO->_wsabuf.len = 1024;
	IO->_wsabuf.buf = IO->_buffer;
	IO->_type = IO->Read;
	IO->clnts = Self;

	int flag = 0;
	WSARecv(mSock, &IO->_wsabuf, 1, NULL, (LPDWORD)&flag, &IO->_overlap, NULL);
}

void ClientData::SetOffLine()
{
	auto Self(shared_from_this());
	mStrand->push([Self] {
		Self->IsOnLine = false;
	});
}


map<SOCKET, shared_ptr<ClientData>> ClientData::Clients;

void YCServer::Init()
{
	printf(" ========== Server Start ==========\n ");
	jobManager = new JobManager();
	GetSystemInfo(&sysInfo);
	SSrd = new Strand(*jobManager);
}

YCServer::~YCServer()
{
	delete jobManager;
}

void YCServer::Srv_Start()
{
	if (!Srv_Startup()) 
	{
		printf("StartUP Erorr\n");
		return;
	}

	Srv_RunWorker();

	for (unsigned int i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
	{
		wThread.push_back(new std::thread([this]()
		{
			SOCKET sock;
			DWORD bytesTrans;
			ClientHandle* CH;
			ClientIO* IO;

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
						
						c->mStrand->push([c, buf, bytesTrans] { c->AsyncRead(buf, bytesTrans); });
						delete IO;
					}
					break;
					case IO->Write:
					{
						printf("[%d][byte:%d] 전송완료!\n%p\n", packetNumber++, IO->_wsabuf.len, IO);
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

	Srv_Accept();

	for (auto i : wThread)			i->join();
	for (auto& i : workerThreads)	i.join();

	closesocket(mServerSock);

	WSACleanup();
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
	for (unsigned int i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
	{
		workerThreads.push_back(std::move(thread([this] { jobManager->run(); })));
	}
}

void YCServer::Srv_Accept()
{
	while (true)
	{
		SOCKET			cSock;
		SOCKADDR_IN		cAddr;
		int addrLen = sizeof(cAddr);

		cSock = accept(mServerSock, (SOCKADDR*)&cAddr, &addrLen);
		bool accept_error =
			cSock == INVALID_SOCKET;

		if (!accept_error)	printf("client connect!\n");
		else				printf("accept error!\n");

		auto ch = (ClientHandle*)malloc(sizeof(ClientHandle));
		ch->_sock = cSock;
		memcpy(&(ch->_addr), &cAddr, addrLen);

		CreateIoCompletionPort((HANDLE)cSock, CP, (ULONG_PTR)ch, 0);


		SSrd->push([cSock, this, ch]
		{
			auto c = Create(cSock);

			c->CH = ch;

			printf("클라이언트 허용 ID : %lld\n", c->mSock);
			c->mStrand->push([cSock, c]
			{ 
				int recvByte = 0, flag = 0;

				auto clientIO = new ClientIO();
				memset(&clientIO->_overlap, 0, sizeof(OVERLAPPED));
				clientIO->_type = clientIO->Read;
				clientIO->_wsabuf.buf = clientIO->_buffer;
				clientIO->_wsabuf.len = BUFSIZ;
				clientIO->clnts = c;
				WSARecv(cSock, &(clientIO->_wsabuf), 1, (LPDWORD)&recvByte, (LPDWORD)&flag, &(clientIO->_overlap), NULL); 
			});
		});
	}
}

void YCServer::Srv_Disconnect(SOCKET sock)
{
	SSrd->push([sock]
	{
		if (ClientData::Clients.find(sock) == ClientData::Clients.end())
		{
			static int i = 0;
			printf("종료 처리 두번 됨[%d]\n", i=!i);
			return;
		}
		printf("클라이언트 접속 종료.\n");

		ClientData::Clients[sock]->SetOffLine();
		ClientData::Clients.erase(sock);
	});
}

shared_ptr<ClientData> YCServer::Create(SOCKET cSock)
{
	auto c = std::make_shared<ClientData>(new Strand(*jobManager), cSock);
	ClientData::Clients[cSock] = c;
	return c;
}



Strand* YCServer::SSrd = nullptr;