#include "pch.h"
#include "JobManager.h"
#include "ThreadPool.h"
#include "YCServer.h"



class P
{
	Strand* s;
public:
	P(JobManager& jm) { s = new Strand(jm); }
};


void Test()
{
	WSAData wsaData;

	HANDLE CP;

	ClientIO* clientIO;
	ClientHandle* clientHandle;

	SOCKET				mServerSock;
	SOCKADDR_IN			sAddr;

	vector<std::thread*> wThread;



	mutex mt;
	vector<ThreadSafeQueue<ClientIO*>*> mSend_q;
	vector<ClientData*> mClnts;

	bool StartUp_error = WSAStartup(MAKEWORD(2, 2), &wsaData) == -1;


	CP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	for (int i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
	{
		wThread.push_back(new std::thread([CP, &mSend_q, &mClnts, &mt]()
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
							printf("메시지 받음\n");

							if (bytesTrans == 0) { printf("클라이언트 접속 종료.\n"); closesocket(sock); free(CH); free(IO); continue; }
							ClientData* cd;
							{
								std::lock_guard<mutex> lock(mt);
								mSend_q[IO->clnts->id]->enqueue(IO);
								cd = IO->clnts;
							}

							//WSASend(sock, &IO->_wsabuf, 1, NULL, 0, &IO->_overlap, NULL);


							IO = (ClientIO*)malloc(sizeof(ClientIO));
							memset(&IO->_overlap, 0, sizeof(OVERLAPPED));
							IO->_wsabuf.len = 1024;
							IO->_wsabuf.buf = IO->_buffer;
							IO->_type = IO->Read;
							IO->clnts = cd;

							int flag = 0;
							WSARecv(sock, &IO->_wsabuf, 1, NULL, (LPDWORD)&flag, &IO->_overlap, NULL);
						}
						break;
						case IO->Write:
						{
							printf("전송완료!\n");
							free(IO);
						}
						break;
						}
					}
					else
					{
						sock = CH->_sock;
						if (bytesTrans == 0) { printf("클라이언트 비정상 접속 종료.\n"); closesocket(sock); free(CH); free(IO); continue; }
					}


				}
			}));
	}

	thread SendThread(
		[&] {
			while (true) {
				std::lock_guard<mutex> lock(mt);

				for (int i = 0; i < mSend_q.size(); i++)
				{
					while (mSend_q[i]->size())
					{
						auto IO = mSend_q[i]->dequeue();
						IO->_type = IO->Write;
						WSASend(mClnts[i]->sock, &IO->_wsabuf, 1, NULL, 0, &IO->_overlap, NULL);
					}
				}
			}
		});

	mServerSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	bool SocketCreate_error = mServerSock == INVALID_SOCKET;

	memset(&sAddr, 0, sizeof(sAddr));
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	sAddr.sin_port = htons(51234);

	bool Bind_error = bind(mServerSock, (SOCKADDR*)&sAddr, sizeof(sAddr)) == -1;
	bool listen_error = listen(mServerSock, 5);


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

		clientHandle = (ClientHandle*)malloc(sizeof(ClientHandle));
		clientHandle->_sock = cSock;
		memcpy(&(clientHandle->_addr), &cAddr, addrLen);

		CreateIoCompletionPort((HANDLE)cSock, CP, (ULONG_PTR)clientHandle, 0);

		// 클라이언트 추가
		// 지금은 락처리 하지만 Strand에 추가할것임.
		{
			std::lock_guard<mutex> lock(mt);
			mClnts.push_back(new ClientData{ cSock, (int)mClnts.size() });
			mSend_q.push_back(new ThreadSafeQueue<ClientIO*>());
		}

		int recvByte = 0, flag = 0;

		clientIO = new ClientIO();
		memset(&clientIO->_overlap, 0, sizeof(OVERLAPPED));
		clientIO->_type = clientIO->Read;
		clientIO->_wsabuf.buf = clientIO->_buffer;
		clientIO->_wsabuf.len = BUFSIZ;
		clientIO->clnts = mClnts.back();

		WSARecv(cSock, &(clientIO->_wsabuf), 1, (LPDWORD)&recvByte, (LPDWORD)&flag, &(clientIO->_overlap), NULL);
	}

	for (auto i : wThread) i->join();
	SendThread.join();

	closesocket(mServerSock);

	WSACleanup();
}

int main()
{
	Test();

	//list<P> pl;

	//JobManager jd;

	//vector<thread> t;
	//ThreadPool tp(4);





	//for (int i = 0; i < 4; i++) t.push_back(thread([&] { jd.run(); }));

 //	atomic<int> n1 = 1;
	//atomic<int> n2 = 1;
	//atomic<int> n3 = 1;
	//atomic<int> n4 = 1;
	//atomic<int> n5 = 1;

	//pl.push_back(P(jd));
	//pl.push_back(P(jd));
	//pl.push_back(P(jd));
	//pl.push_back(P(jd));
	//pl.push_back(P(jd));

	//for (int i = 0; i < 10000; i++)
	//{
	//	jd.add_Job(0, [&] { n1++; if (n1 == 10000) printf("1\n"); });
	//	jd.add_Job(1, [&] { n2++; if (n2 == 10000) printf("2\n"); });
	//	jd.add_Job(2, [&] { n3++; if (n3 == 10000) printf("3\n"); });
	//	jd.add_Job(3, [&] { n4++; if (n4 == 10000) printf("4\n"); });
	//	jd.add_Job(4, [&] { n5++; if (n5 == 10000) printf("5\n"); });
	//}

	//pl.push_back(P(jd));
	//pl.push_back(P(jd));
	//pl.push_back(P(jd));
	//pl.push_back(P(jd));
	//pl.push_back(P(jd));

	////jd.StopThread();
	//for (auto& i : t) i.join();
}