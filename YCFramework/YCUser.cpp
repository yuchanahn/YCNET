#include "pch.h"
#include "YCUser.h"
#include "YCServer.h"
#include "YCSync.h"
#include "YCSend.h"

void YCUser::In(unsigned char* buf, int size)
{
	mReadManager.read(buf, size, ID);
}





YCUser::~YCUser()
{
	delete mSyncer;
	delete mSender;
}
