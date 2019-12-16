#pragma once
#define _SCL_SECURE_NO_WARNINGS
#include <algorithm>
#include <memory>

enum ePacket_Type : int
{
	base,
	transfrom,
	player,
	ping,
};
/*
#define Pack(T,packetT)																											\
struct p##T																														\
{																																\
	ePacket_Type t = packetT;																									\
	T data;																														\
};																																\
union pack##T																													\
{																																\
	pack##T(){ memset( this, 0, sizeof( pack##T ) ); };																			\
	~pack##T(){}																												\
	p##T packet;																												\
	char raw[sizeof(p##T)];																										\
};			
*/
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
HeaderTail get_header_tail(char* data, int len);


struct RawData
{
	bool success;
	
	char* data;
	int dataLen;

	char* lastData;
	int lastDataLen;
};

RawData get_header_tail_to_data(char* data, int len);





/*
struct Base
{
};
Pack(Base,base)
*/
/*
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


*/


/*
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
Pack(PlayerData, player)



struct Ping
{
	long long time;
};
Pack(Ping, ping)
*/


int Read(char*& data, int& len);



/*
void i()
{
	pPlayerData playerTh { };
	playerTh.data.Hp = 100;
	playerTh.data.Mp = 10;

	char* buf;
	int len;

	pTransfromData Tr{};
	Tr.data.x = -3;
	Tr.data.y = 3;

	char* buf2;
	int len2;

	YCPacketBuilder::pack<packPlayerData, pPlayerData>(playerTh, buf, len);

	YCPacketBuilder::pack<packTransfromData, pTransfromData>(Tr, buf2, len2);

	auto Fainal_packet1 = get_header_tail(buf, len);
	auto Fainal_packet2 = get_header_tail(buf2, len2);
	int Fainal_Len = Fainal_packet1.len + Fainal_packet2.len;

	char* Fainal_packet = new char[Fainal_Len];
	std::copy(Fainal_packet1.data, Fainal_packet1.data + Fainal_packet1.len, Fainal_packet);
	std::copy(Fainal_packet2.data, Fainal_packet2.data + Fainal_packet2.len, &Fainal_packet[Fainal_packet1.len]);


	bool read = false;
	int cur_read_len = 0; // 지금까지 읽어드린 길이.


	int test_i = 0;
	while (!read)
	{
		test_i++;
		if		(test_i == 1)	cur_read_len += sizeof(int) - 1;        // 1 - test
		else if (test_i == 2)	cur_read_len += len / 2;                // 2 - test
		else if (test_i == 3)	cur_read_len += len + len2;				// 3 - test
	
		Read(Fainal_packet, cur_read_len);
	}

}
*/