#include "pch.h"

YCTime::YCTime()
{
}

YCTime::~YCTime()
{
}

void YCTime::UpdateDeltaTime()
{
	if (Instance().lastT == nullptr)
	{
		Instance().lastT = new std::chrono::system_clock::time_point;
		return;
	}

	deltaTime = ((std::chrono::duration<float>)(std::chrono::system_clock::now() - *Instance().lastT)).count();
	*Instance().lastT = std::chrono::system_clock::now();
}

void YCTime::TimerStart()
{
	t = std::chrono::system_clock::now();
}

float YCTime::TimerEnd()
{
	return ((std::chrono::duration<float>)(std::chrono::system_clock::now() - t)).count();
}


float YCTime::deltaTime = 0;
int YCTime::WriteTime = 0;
