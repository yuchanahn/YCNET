#pragma once
#define _SCL_SECURE_NO_WARNINGS
#include <algorithm>
#include <memory>

enum ePacket_Type : int
{
	base,
	transfrom,
	player,
};

#define Pack(T,packetT)																											\
struct p##T																														\
{																																\
	static ePacket_Type t;																										\
	T data;																														\
};																																\
ePacket_Type p##T::t= packetT;																									\
union pack##T																													\
{																																\
	pack##T(){ memset( this, 0, sizeof( pack##T ) ); };																			\
	~pack##T(){}																												\
	p##T packet;																												\
	char raw[sizeof(p##T)];																										\
};			

union charCenvrt
{
	int integer;
	char char_integer[sizeof(int)];
};


struct HeaderTail
{
	char* data;
	int len;
};

// Header : Tail을 포함한 한 패킷의 길이 데이터.
// Tail	  : UnPack 할 수 있는 데이터.
HeaderTail get_header_tail(char* data, int len)
{
	HeaderTail raw_packet;
	raw_packet.data = new char[sizeof(int) + len];
	raw_packet.len = sizeof(int) + len;

	charCenvrt cc;
	cc.integer = raw_packet.len;


	std::copy(cc.char_integer, cc.char_integer + sizeof(int), raw_packet.data);
	std::copy(data, data+len, &raw_packet.data[sizeof(int)]);
	
	return raw_packet;
}

struct RawData
{
	bool success;
	
	char* data;
	int dataLen;

	char* lastData;
	int lastDataLen;
};

RawData get_header_tail_to_data(char* data, int len)
{
	if (len < sizeof(int)) return RawData{ };

	RawData d {};
	int ReadDataLen = 0;


	charCenvrt cc;
	std::copy(data, data + sizeof(int), cc.char_integer);						// 헤더에 써있는 길이.
	
	ReadDataLen = cc.integer;
	
	if(len < ReadDataLen) return RawData{ };

	d.dataLen = ReadDataLen - sizeof(int);									// 헤더의 길이만큼 빼주기.
	d.data = new char[d.dataLen];
	std::copy(data + sizeof(int), data + (d.dataLen + sizeof(int)), d.data);

	if ((len - ReadDataLen) > 0) 
	{
		d.lastDataLen = len - ReadDataLen;
		d.lastData = new char[d.lastDataLen];
		std::copy(data + ReadDataLen, data + ReadDataLen + d.lastDataLen, d.lastData);
	}
	else
	{
		d.lastDataLen = 0;
	}
	char bb[12];
	std::copy(data, data + 12, bb);
	d.success = true;

	return d;
}





struct Base
{
};
Pack(Base,base)


class YCPacketBuilder
{
	YCPacketBuilder() {}
public:
	template <class T, class T2>
	static void pack(T2 data, char*& byte_data, int& len)
	{
		T pData;
		len = sizeof(T);
		pData.packet = data;
		byte_data = new char[len];
		std::copy(pData.raw, pData.raw + len, byte_data);
	}

	template <class T, class T2>
	static T2 unpack(char* data, int len)
	{
		T p;
		std::copy(data, data+len, p.raw);
		return p.packet;
	}

	static int GetPacketType(char* data)
	{
		return unpack<packBase, pBase>(data, sizeof(packBase)).t;
	}
};





struct TransfromData
{
	int x;
	int y;
};
Pack(TransfromData, transfrom)

struct PlayerData
{
	int Hp;
	int Mp;
	TransfromData Tr;
	int Str;
	int Dex;
	int Cri;
	int Int;
};

void i()
{
	pTransfromData playerTh { };
	playerTh.data.x = 11;
	playerTh.data.y = 99;

	char* buf;
	int len;

	YCPacketBuilder::pack<packTransfromData, pTransfromData>(playerTh, buf, len);

	auto Fainal_packet = get_header_tail(buf, len);


	bool read = false;
	int cur_read_len = 0; // 지금까지 읽어드린 길이.


	int test_i = 0;
	while (!read)
	{
		test_i++;
		if		(test_i == 1)	cur_read_len += sizeof(int) - 1;        // 1 - test
		else if (test_i == 2)	cur_read_len += len / 2;                // 2 - test
		else if (test_i == 3)	cur_read_len += len;					// 3 - test
		
		auto obj = get_header_tail_to_data(Fainal_packet.data, cur_read_len);
		if (obj.success)
		{
			auto p = ((pTransfromData*)(&(obj.data)));
			printf("x:%d, y:%d [packet len : %d]\n", p->data.x, p->data.y, obj.dataLen);
			delete obj.data;

			read = true;
			delete Fainal_packet.data;
			
			if (obj.lastData > 0) 
			{
				Fainal_packet.data = obj.lastData;
				Fainal_packet.len = obj.lastDataLen;
			}
			else
			{
				Fainal_packet.data = nullptr;
				Fainal_packet.len = 0; 
				cur_read_len = 0;
			}
		}
	}

}