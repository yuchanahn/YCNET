#pragma once
class UserBase
{
	struct fplayerT* PlayerData;
	function<void(char* data, int len)> mWriteFunc;
	function<void(char* data, int len)> mWriteAllFunc;

	void Init();
public:
	UserBase(function<void(char* data, int len)> WriteFunc, function<void(char* data, int len)> all) :mWriteAllFunc(all), mWriteFunc(WriteFunc) { Init(); }
	void Write__(char* data, int len);
	int ID;
	bool boom = false;

	void __ping(struct fpingT* data);



	map<int, function<void(char* data, int len)>> PacketEvent;
};