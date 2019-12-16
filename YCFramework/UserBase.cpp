#include "pch.h"

#include "UserBase.h"
#include "YCPacket.h"
#include "YCServer.h"
#include "JobManager.h"
#include "Base_generated.h"

void UserBase::Init()
{
	PlayerData = new fplayerT();
	PlayerData->name = "none";
	PacketEvent[eFB_Type::eFB_Type_fping] = [this](char* data, int len) 
	{
		PackData packData(data, len);
		__ping(packData.Get<fping>());
		//auto p = YCPacketBuilder::unpack<packPing, pPing>(data, len);
		//__ping(&p.data);
	};



	PacketEvent[eFB_Type::eFB_Type_fplayer] = [this](char* data, int len)
	{
		PackData packData(data, len);
		auto future_player = packData.Get<fplayer>();
		future_player->id = ID;
		future_player->str = boom ? 1 : 0;
		flatbuffers::FlatBufferBuilder fbb;
		fbb.Finish(fplayer::Pack(fbb, future_player));
		auto ht = get_header_tail((char*)fbb.GetBufferPointer(), fbb.GetSize());

		mWriteAllFunc(ht.data, ht.len);
		delete future_player;
	};

	PacketEvent[eFB_Type::eFB_Type_fid] = 
	[this](char* data, int len)
	{
		PackData packData(data, len);
		auto fut = packData.Get<fid>();
		fut->id = ID;
		flatbuffers::FlatBufferBuilder fbb;
		fbb.Finish(fid::Pack(fbb, fut));
		auto ht = get_header_tail((char*)fbb.GetBufferPointer(), fbb.GetSize());

		Write__(ht.data, ht.len);
		delete fut;
	};
}



void UserBase::Write__(char* data, int len)
{
	mWriteFunc(data, len); // data ¾Ë¾Æ¼­ delet ÇØÁÜ.
}

void UserBase::__ping(fpingT* data)
{
	//printf("[ping : %lld]\n", data->time);
	flatbuffers::FlatBufferBuilder fbb;
	fbb.Finish(fping::Pack(fbb, data));

	auto ht = get_header_tail((char*)fbb.GetBufferPointer(), fbb.GetSize());
	Write__(ht.data, ht.len);
	// Write to This Client;

	//PackData* packData = new PackData(Self->mBuf_len);

	/*

	pPing pack_ping;
	pack_ping.data = *data;


	char* buf;
	int len;
	YCPacketBuilder::pack<packPing, pPing>(pack_ping, buf, len);

	auto ht = get_header_tail(buf, len);
	delete[] buf;

	char b[20];
	std::copy(ht.data, ht.data + ht.len, b);
	Write__(ht.data, ht.len);


	*/
}