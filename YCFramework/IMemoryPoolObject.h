#pragma once
__interface IMemoryPoolObject
{
public:
	// ������.
	void Begin() = 0;
	// �Ҹ���.
	void End() = 0;
};

