#include "pch.h"

#include "YCServer.h"
#include "YCUser.h"
#include "YCSync.h"
#include "YCSend.h"



void YCServer::Init()
{
	yc::log("Server Start!");
	mJobManager = new JobManager();
	Job = new Strand(*mJobManager);
}

YCServer::~YCServer()
{
	delete mJobManager;
}

void YCServer::Srv_Start()
{
	YCTime::UpdateDeltaTime();
	if (!Srv_Startup())
	{
		printf("Start() : StartUP Erorr\n");
		return;
	}

	Srv_RunWorker();

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	for (unsigned int i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
	{
		IOCP_Thread.push_back(std::thread([this]() {
			SOCKET sock;
			DWORD Len;
			ClientHandle* CH;
			IOData* IO;

			while (true)
			{
				if (GetQueuedCompletionStatus(CP, &Len, (PULONG_PTR)&CH, (LPOVERLAPPED*)&IO, INFINITE))
				{
					sock = CH->mSock;

					// 나중에 IO데이터를 클라이언트 안에 생성해서 재사용 하자!
					// 클라이언트 자체가 풀링이 된다!!

					switch (IO->mType)
					{
					case IO->In:
					{
						auto c = &mSessions[Sessions_ID_Mapping[sock]];
						if (Len == 0)
						{
							// 모든 Strand를 돌고 안전하게 종료.
							c->job->AddLate([this, c, IO, sock] {
								Client_Disconnect(sock);
								delete IO;
							});
							continue;
						}
						c->job->Add([c, IO, Len]() {
							c->user->In((unsigned char*)IO->_buffer, Len);
							delete IO;

							auto io = new IOData;
							memset(&io->_overlap, 0, sizeof(OVERLAPPED));
							io->_wsabuf.len = 1024;
							io->_wsabuf.buf = io->_buffer;
							io->mType = io->In;
							int flag = 0;
							WSARecv(c->socket_id, &io->_wsabuf, 1, NULL, (LPDWORD)&flag, &io->_overlap, NULL);
						});
					}
					break;
					case IO->Out:
					{
						delete IO;
					}
					break;
					}
				}
				else
				{
					sock = CH->mSock;
					if (Len == 0)
					{
						auto c = &mSessions[Sessions_ID_Mapping[sock]];
						c->job->AddLate([this, c, IO, sock] {
							Client_Disconnect(sock);
							delete IO;
						});
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

	YCTime::UpdateDeltaTime();
	Job->Add([this] { Update(); });

	Srv_Accept();

	for (auto& i : IOCP_Thread)		i.join();
	for (auto& i : workerThreads)	i.join();

	closesocket(mServerSock);

	WSACleanup();
}
void YCServer::Update()
{
	YCTime::UpdateDeltaTime();

	loop();

	Job->AddLate([this] { Update(); });
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
		if (clientSocket == INVALID_SOCKET)	assert("accept Error");

		Job->Add([cAddr, addrLen, clientSocket, this] {
			int client_id = -1;
			if (!mFree_Sessions_ID_Stack.empty())
			{
				auto id_ptr = mFree_Sessions_ID_Stack.pop();
				client_id = *id_ptr;
				delete id_ptr;
				mSessions[client_id].socket_id = clientSocket;
				memset(mSessions[client_id].client_handle, 0, sizeof(ClientHandle));
			}
			else
			{
				client_id = mSessions.size();
				auto strand = new Strand(*mJobManager);
				mSessions.push_back(
					YCSession_t
					{
						client_id,
						clientSocket,
						strand,
						new ClientHandle,
						new YCUser(new YCSync {strand}, new YCSend(clientSocket), client_id),
					});
			}
			auto& ch = mSessions[client_id].client_handle;
			ch->mSock = clientSocket;
			Sessions_ID_Mapping[clientSocket] = client_id;
			memcpy(&(ch->_addr), &cAddr, addrLen);

			CreateIoCompletionPort((HANDLE)clientSocket, CP, (ULONG_PTR)ch, 0);
			auto& client = mSessions[client_id];
			connect_ev(client_id);
			client.job->Add([clientSocket, client] {
				int recvByte = 0, flag = 0;

				auto clientIO = new IOData();
				memset(&clientIO->_overlap, 0, sizeof(OVERLAPPED));
				clientIO->mType = clientIO->In;
				clientIO->_wsabuf.buf = clientIO->_buffer;
				clientIO->_wsabuf.len = BUFSIZ;

				WSARecv(
					clientSocket,
					&(clientIO->_wsabuf),
					1,
					(LPDWORD)&recvByte,
					(LPDWORD)&flag,
					&(clientIO->_overlap),
					NULL);
				});
		});
	}
}
void YCServer::Client_Disconnect(SOCKET sock)
{
	Job->Add([this, sock] {
		//이미 삭제되었다.
		if (Sessions_ID_Mapping.find(sock) == Sessions_ID_Mapping.end()) return;
		auto id = Sessions_ID_Mapping[sock];
		disconnect_ev(id);
		closesocket(sock);
		Sessions_ID_Mapping.erase(sock);
		mFree_Sessions_ID_Stack.push(new int(id));
	});
}

bool ThreadData::GetNewStrand()
{
	auto size = FutureJobs.size();
	for (unsigned int i = 0; i < size; ++i)
	{
		if (!FutureJobs[i].empty())
		{
			CurrentJobs[i] = std::move(FutureJobs[i]);
			if (strandID != -1) IsCurrentJob[strandID] = false;
			strandID = i;
			IsCurrentJob[i] = true;
			return true;
		}
	}
	return false;
}
int ThreadData::GetCurrentStrandID()
{
	return strandID;
}

void JobManager::Init()
{
}
void JobManager::add_Job_Late(int id, std::function<void()> f)
{
	std::unique_lock<std::mutex> lock(mt);
	if (Stop) throw std::runtime_error("enstd::queue on stopped ThreadPool");

	if (isCurrentJob[id])	mCurrentJobs[id].push(std::move(f));
	else					mFutureJobs[id].push(std::move(f));
	c.notify_one();
}
void JobManager::add_Job(int id, std::function<void()> f)
{
	std::unique_lock<std::mutex> lock(mt);

	bool Trigger = false;

	if (Stop) throw std::runtime_error("enstd::queue on stopped ThreadPool");
	auto t_id = std::this_thread::get_id();
	bool cid = (mThreadDataMap.find(t_id) != mThreadDataMap.end());

	if (cid && mThreadDataMap[t_id]->GetCurrentStrandID() == id)
	{
		Trigger = true;
	}
	else
	{
		if (isCurrentJob[id])	mCurrentJobs[id].push(std::move(f));
		else					mFutureJobs[id].push(std::move(f));
	}
	lock.unlock();

	if (Trigger)
	{
		f();
		return;
	}
	c.notify_one();
}
int JobManager::create_Strand_ID()
{
	std::lock_guard<std::mutex> lock(mt);
	static int strand = 0;
	mCurrentJobs.push_back(std::queue <std::function<void()>>());
	mFutureJobs.push_back(std::queue <std::function<void()>>());
	isCurrentJob.push_back(false);
	return strand++;
}
void JobManager::clear(int m_id)
{
	std::lock_guard<std::mutex> lock(mt);

	while (!mCurrentJobs[m_id].empty())
	{
		mCurrentJobs[m_id].pop();
	}
	while (!mFutureJobs[m_id].empty())
	{
		mFutureJobs[m_id].pop();
	}
	isCurrentJob[m_id] = false;
}
void JobManager::run()
{
	ThreadData self(mCurrentJobs, mFutureJobs, isCurrentJob);
	mThreadDataMap[std::this_thread::get_id()] = &self;

	while (true)
	{
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(mt);
			c.wait(lock, [&] { return Stop || (self.IsTasking ? true : self.GetNewStrand()); });
			if (Stop) return;
			task = std::move((self.Strand)->front());
			(self.Strand)->pop();
			lock.unlock();
			task();
		}
	}
}
void JobManager::StopThread()
{
	std::lock_guard<std::mutex> lock(mt);
	Stop = true;
}

void Strand::Init()
{
	id = jm.create_Strand_ID();
}
void Strand::Add(std::function<void()> job)
{
	jm.add_Job(id, job);
}
void Strand::AddLate(std::function<void()> job)
{
	jm.add_Job_Late(id, job);
}