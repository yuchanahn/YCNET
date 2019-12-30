#pragma once
__interface IMemoryPoolObject
{
public:
	// 持失切.
	void Begin() = 0;
	// 社瑚切.
	void End() = 0;
};

