#ifndef PCH_H
#define PCH_H

#pragma comment(lib, "ws2_32.lib")


#include <iostream>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <vector>
#include <Ws2tcpip.h> 
#include <mutex>
#include <string.h>
#include <map>
#include <list>
#include <queue>
#include <Windows.h>
#include <utility>
#include <functional>
//#include "Packet.h"

//===============================================================================================================
//		## 크리티컬 섹션 이니셜라이즈 해서 쓰기.
//===============================================================================================================


class Cri_Lock
{
	CRITICAL_SECTION* mCs;
public:
	Cri_Lock(CRITICAL_SECTION* cs)
	{
		mCs = cs;
		EnterCriticalSection(mCs);
	}
	~Cri_Lock()
	{
		LeaveCriticalSection(mCs);
	}
	operator bool()
	{
		return mCs != nullptr;
	}
};


using std::string;
using std::map;
using std::list;
using std::queue;
using Sprite = std::pair<std::string, COLOR16>;
using std::function;
using std::vector;

#define GET(T,member) const T& Get##member() const { return member; }
#define SET(T, member) void Set##member(const T & value) { member =value; }
#define GETSET(T, member) const T& Get##member() const { return member; } void Set##member(const T & value) { member =value; }

#define Lock(cs) if (Cri_Lock lock{cs})
#define wVersion MAKEWORD(2,2)

template<class T> inline void destroy(T*& p) { delete p; p = 0; }

#endif //PCH_H