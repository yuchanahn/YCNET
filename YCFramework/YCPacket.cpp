#include "pch.h"

#include "YCPacket.h"

HeaderTail get_header_tail(char* data, int len)
{
	HeaderTail raw_packet;
	raw_packet.data = new char[sizeof(int) + len];
	raw_packet.len = sizeof(int) + len;

	charCenvrt cc;
	cc.integer = raw_packet.len;


	std::copy(cc.char_integer, cc.char_integer + sizeof(int), raw_packet.data);
	std::copy(data, data + len, &raw_packet.data[sizeof(int)]);

	return raw_packet;
}

RawData get_header_tail_to_data(char* data, int len)
{
	if (len < sizeof(int)) return RawData{ };

	RawData d{};
	int ReadDataLen = 0;


	charCenvrt cc;
	std::copy(data, data + sizeof(int), cc.char_integer);						// 헤더에 써있는 길이.

	ReadDataLen = cc.integer;

	if (len < ReadDataLen) return RawData{ };

	if (ReadDataLen > 1024 || ReadDataLen <= 0)
	{
		printf("[Error] 받을 수 없는 패킷을 받음.\n");
		exit(1);
	}

	d.dataLen = ReadDataLen - sizeof(int);									// 헤더의 길이만큼 빼주기.
	d.data = new char[d.dataLen];
	std::copy(data + sizeof(int), data + (d.dataLen + sizeof(int)), d.data);

	if ((len - ReadDataLen) > 0)
	{
		d.lastDataLen = len - ReadDataLen;
		d.lastData = new char[d.lastDataLen];
		auto _data = (data + ReadDataLen);
		std::copy(_data, _data + d.lastDataLen, d.lastData);
	}
	else
	{
		d.lastDataLen = 0;
	}
	d.success = true;

	return d;
}
/*
int Read(char*& data, int& len)
{

	auto obj = get_header_tail_to_data(data, len);
	int Falinal_Len = 0;
	if (obj.success)
	{
		auto type = YCPacketBuilder::GetPacketType(obj.data);

		if (type == 2)
		{
			auto p = YCPacketBuilder::unpack<packPlayerData, pPlayerData>(obj.data, obj.dataLen).data;

			printf("HP:%d, MP:%d [packet len : %d, type : %d]\n", p.Hp, p.Mp, obj.dataLen, type);
			delete[] obj.data;
			delete[] data;
		}
		else if (type == 1)
		{
			auto p = YCPacketBuilder::unpack<packTransfromData, pTransfromData>(obj.data, obj.dataLen).data;

			printf("x:%d, y:%d [packet len : %d, type : %d]\n", p.x, p.y, obj.dataLen, type);
			delete[] obj.data;
			delete[] data;
		}
		else if (type == 3)
		{
			auto p = YCPacketBuilder::unpack<packPing, pPing>(obj.data, obj.dataLen).data;

			p.time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - p.time;

			printf("ping : %lld [packet len : %d, type : %d]\n", p.time, obj.dataLen, type);
			delete[] obj.data;
			delete[] data;
		}

		if (obj.lastDataLen > 0)
		{
			data = obj.lastData;
			Falinal_Len = obj.lastDataLen;
		}
		else
		{
			data = nullptr;
			len = 0;
			Falinal_Len = 0;
		}
	}
	else
	{
		Falinal_Len = -1;
	}
	return Falinal_Len;

}
*/