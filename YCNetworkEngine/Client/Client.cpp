//#include "pch.h"
//
//YC_Client::YC_Client(const char* IP, u_short PORT)
//{
//	InitializeCriticalSection(&g_cs);
//	g_NetWork = this;
//	PacketBuilder::InitPackets();
//
//	if (WSAStartup(wVersion, &wsaData) != 0)
//	{
//		printf("WSAStartup error");
//	}
//
//	sock = socket(PF_INET, SOCK_STREAM, 0);
//	if (sock == INVALID_SOCKET)
//	{
//		printf("socket error");
//	}
//
//	memset(&sAddr, 0, sizeof(sAddr));
//	sAddr.sin_family = PF_INET;
//	sAddr.sin_port = htons(PORT);
//	sAddr.sin_addr.s_addr = inet_addr(IP);
//
//	if (connect(sock, (SOCKADDR*)&sAddr, sizeof(sAddr)) == SOCKET_ERROR)
//	{
//		printf("connect error\n");
//	}
//
//	auto Th = new std::thread([this]()
//		{
//			char buf[1024];
//			char cbuf[1024];
//
//			int clen = 0;
//
//			int head = 0;
//
//			//==============================================================================================
//			//			# 읽기 쓰레드 :  읽기 쓰레드는 무조건 하나여야함.
//			//==============================================================================================
//			while (true)
//			{
//				int len = recv(sock, buf, 1024, 0);
//				if (len <= -1)
//				{
//					memset(cbuf, 0, 1024); clen = 0; continue;
//				}
//
//				memcpy_s(&cbuf[clen], len, buf, len);
//				clen += len;
//
//				if (clen < 4) { continue; }
//				memcpy_s(&head, 4, cbuf, 4);
//				while (clen >= head)
//				{
//					AsnycRead(&cbuf[4], head - 4);
//					clen -= head;
//					if (clen < 4) break;
//					memcpy_s(&head, 4, &cbuf[clen], 4);
//
//					if (clen == 0)
//					{
//						memset(cbuf, 0, 1024);
//					}
//				}
//			}
//		});
//	while (true)
//	{
//		Update();
//	}
//
//	char sendbuffer[1024] = { 0 };
//
//	closesocket(sock);
//	WSACleanup();
//}
//
//void YC_Client::AsnycRead(char* data, int len)
//{
//	char* d = new char[len];
//	memcpy_s(d, len, data, len);
//	Lock(&g_cs) mWorkerQ.push([d, len]()
//		{
//			ReadManager::Read(d, len);
//			delete d;
//		});
//
//}
//
//void YC_Client::AsnycWrite(char* data, int len)
//{
//	char sendbuffer[1024] = { 0 };
//	len += 4;
//	int header = len;
//	memcpy_s(sendbuffer, 4, &header, 4);
//	memcpy_s(&sendbuffer[4], header, data, header);
//	send(sock, sendbuffer, len, 0);
//}
//
//void YC_Client::Update()
//{
//	GameObject::Init();
//	Scene::LoadScene(new CharSelScene());
//	bool b = true;
//
//	list<function<void()>> jabList;
//
//	while (GameObject::Rander())
//	{
//		Lock(&g_cs)
//		{
//			while (!mWorkerQ.empty())
//			{
//				jabList.push_back(mWorkerQ.front());
//				mWorkerQ.pop();
//			}
//		}
//		while (!jabList.empty())
//		{
//			jabList.front()();
//			jabList.pop_front();
//		}
//		Input::KeyReset();
//	}
//
//	GameObject::Release();
//}
//
//YC_Client* YC_Client::g_NetWork = 0;