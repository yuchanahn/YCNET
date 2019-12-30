#pragma once
class YCSession
{
private:
	char* mBuffer;
	int mBuf_len;
	ClientHandle* CH;
	SOCKET mSocket;

private:
	void Init();

public:
	class Strand* Job;

public:
	YCSession(class Strand* srd, SOCKET sock) :
		Job(srd), mSocket(sock), mBuffer(nullptr), mBuf_len(0)
	{
		Init();
	};
	~YCSession();

	void Read(char* data, int len);
	void Write(char* data, int len);
};