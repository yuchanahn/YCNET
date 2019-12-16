#include "pch.h"

#include "YCServer.h"
#include "JobManager.h"
#include "YCPacket.h"
#include "UserBase.h"
#include "Base_generated.h"
#include <time.h>


ClientData::ClientData(Strand* strand, SOCKET sock)
{
	srand(time(0));
	mStrand = strand;
	this->mSock = sock;
	IsOnLine = true;

	
	User = new UserBase([this] (char* data, int len) 
	{
		StrandAsyncWrite(data, len);
	},
	[this](char* data, int len)
	{
		StrandAsyncWriteAll(data, len);
	}
	);
	User->ID = mSock;
}

ClientData::~ClientData()
{
	// 소켓을 바로 닫지 않아서 이상한 일이 발생하는듯.
	printf("[ sock_id : %lld ]클라이언트 종료!\n", mSock);
	closesocket(mSock);
	free(CH);
	if (!mStrand) delete mStrand;
}

void ClientData::StrandAsyncWrite(char* data, int len)
{
	auto Self(shared_from_this());
	mStrand->push(
	[Self, data, len] {
		Self->AsyncWrite(data, len);
		delete[] data;
	});

}

void ClientData::StrandAsyncWriteAll(char* data, int len)
{
	auto Self(shared_from_this());
	mStrand->push(
	[Self, data, len] 
	{
		YCServer::SSrd->push(
		[Self, data, len] {
			for (auto i : Clients)
			{
				char* _data = new char[len];
				std::copy(data, data + len, _data);
				i.second->StrandAsyncWrite(_data, len);
			}
			delete[] data;
		});
	});

}


void ClientData::Boomed() // 폭탄이면 자살해라.
{
	auto Self(shared_from_this());
	mStrand->push([Self] {
		if (Self->User->boom) Self->SetOffLine();
	});
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
	printf("전송할 데이터 [size : %d]\n", IO->_wsabuf.len);

	WSASend(mSock, &IO->_wsabuf, 1, NULL, 0, &IO->_overlap, NULL);
}

void ClientData::AsyncRead(char* data, int len)
{
	/// 패킷에 따른 처리...

	///

	// =====================================================================
	//		## 동기화 관련이슈 있을 수도있다.
	// =====================================================================
	shared_ptr<ClientData> Self(shared_from_this());
	mStrand->push(
	[Self, data, len]
	{
		
		auto PrevBuf = Self->mBuffer;
		Self->mBuffer = new char[Self->mBuf_len + len];

		if (PrevBuf != nullptr) 
		{
			std::copy(PrevBuf, PrevBuf + Self->mBuf_len, Self->mBuffer); 
			delete[] PrevBuf;
		}
		std::copy(data, data + len, Self->mBuffer + Self->mBuf_len);
		delete[] data;
		Self->mBuf_len += len;
		
		while (true)
		{
			auto obj = get_header_tail_to_data(Self->mBuffer, Self->mBuf_len);
			if (obj.success)
			{
				PackData packData(obj.data, obj.dataLen);

				auto& PE = Self->User->PacketEvent;
				auto baseCastObj = packData.Get<Base>();if (PE.find(baseCastObj->fType) == PE.end())
				{
					printf("알수 없는 패킷이 도착함\n");
					return;
				}

				PE[baseCastObj->fType](obj.data, obj.dataLen);


				delete baseCastObj;
				delete[] obj.data;
				delete[] Self->mBuffer;

				if (obj.lastDataLen > 0)
				{
					Self->mBuffer = obj.lastData;
					Self->mBuf_len = obj.lastDataLen;
				}
				else
				{
					Self->mBuffer = nullptr;
					Self->mBuf_len = 0;
					break;
				}
			}
			else
			{
				break;
			}
		}
	});

	// =====================================================================
	//		## 외부 함수로 빼는 예제.
	// =====================================================================
	/*
	mStrand->push([Self, data, len]
	{
		auto pr = Self->mBuffer;
		Self->mBuffer = new char[Self->mBuf_len + len];
		
		std::copy(pr, pr + Self->mBuf_len, Self->mBuffer);
		std::copy(data, data + len, Self->mBuffer+ Self->mBuf_len);

		delete[] data;
		Self->mBuf_len += len;

		Read(Self->mBuffer, Self->mBuf_len);
	});
	*/


	// =====================================================================
	//		## 전체 보내기 예제.
	// =====================================================================
	/*
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
	*/

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

	mStrand->push([Self] 
	{
		foutT o;
		o.fType = eFB_Type::eFB_Type_fout;
		o.id = Self->mSock;
		flatbuffers::FlatBufferBuilder fbb;
		fbb.Finish(fout::Pack(fbb,&o));
		auto ht = get_header_tail((char*)fbb.GetBufferPointer(), fbb.GetSize());

		YCServer::SSrd->push([ht]
			{
				for (auto& i : Clients)
				{
					auto _data = new char[ht.len];
					std::copy(ht.data, ht.data + ht.len, _data);

					i.second->mStrand->push([i, _data, ht]
						{
							i.second->AsyncWrite(_data, ht.len);
							delete[] _data;
						});
				}
				delete[] ht.data;
			});
		
		Self->IsOnLine = false;
	});
}


map<SOCKET, shared_ptr<ClientData>> ClientData::Clients;

void YCServer::Init()
{
	printf(" ========== Server Start ==========\n");
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
	Time::UpdateDeltaTime();
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
	Time::UpdateDeltaTime();
	SSrd->push([this] { Update(); });
	Srv_Accept();

	for (auto i : wThread)			i->join();
	for (auto& i : workerThreads)	i.join();

	closesocket(mServerSock);

	WSACleanup();
}



void YCServer::Update()
{
	Time::UpdateDeltaTime();

	static float dt = 0;
	static float dt2 = 0;
	static float dt3 = 0;
	static size_t FPS = 0;

	dt += Time::deltaTime;
	FPS++;
	if (dt > 1)
	{
		dt = 0;
		printf("Server FPS : %lld\n", FPS);
		FPS = 0;
	}

	static float gen = 20;
	static float genT = 0;
	static int MobIDOffset = 0;

	genT += Time::deltaTime;
	if (genT > gen)
	{
		genT = 0;
		auto m = new fmobT();
		m->id = MobIDOffset;
		mobs[MobIDOffset++] = m;
		m->hp = 100;
		m->fType = eFB_Type::eFB_Type_fmob;
		m->x = rand() % 21 - 10;
		m->y = rand() % 21 - 10;
	}

	dt2 += Time::deltaTime;
	if (dt2 > 0.3f)
	{
		dt2 = 0;
		for (auto i : mobs)
		{
			Vec2f v2((rand() % 3) - 1, (rand() % 3) - 1);
			i.second->x += v2.x * 0.3f * ((rand() % 3) + 1);
			i.second->y += v2.y * 0.3f * ((rand() % 3) + 1);

			flatbuffers::FlatBufferBuilder fbb;
			fbb.Finish(fmob::Pack(fbb, i.second));
			auto ht = get_header_tail((char*)fbb.GetBufferPointer(), fbb.GetSize());

			for (auto& i : ClientData::Clients)
			{
				auto _data = new char[ht.len];
				std::copy(ht.data, ht.data + ht.len, _data);
				i.second->StrandAsyncWrite(_data, ht.len);
			}
		}
	}


	

	SSrd->push([this] { Update(); });
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
				c->User->boom = c->Clients.size() == 1;
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

Base* PackData::m_GetBase(char* data_)
{
	return (Base*)GetBase(data_);
}