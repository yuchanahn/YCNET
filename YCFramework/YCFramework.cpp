#include "pch.h"



class Jabs
{
public:

	bool Used = false;
	ThreadSafeQueue<function<void()>> mJabs;
};

int a = 0;

struct WaitThread
{
	int num;
};

class JabPool
{
	std::mutex m; 
	std::condition_variable c;
	bool ready = false;
public:
	unordered_map<void*, Jabs> worker;
	void Add(void* id, function<void()> item)
	{
		std::lock_guard<std::mutex> lock(m);
		worker[id].mJabs.enqueue(item);
		if (!worker[id].Used) 
		{
			ready = true; 
			c.notify_one(); 
		}
	}

	void Run()
	{
		std::unique_lock<std::mutex> lock(m);
		c.wait(lock, [this] { return ready; });
		ready = false;

		bool val = false;
		Jabs* jabs = nullptr;
		for (auto& i : worker)
		{
			if (!i.second.Used) 
			{
				val = true;
				i.second.Used = true;
				jabs = &i.second;
				break;
			}
		}
		lock.unlock();
		if(val) Work(*jabs);
		Run();
	}

	void Work(Jabs& jabs)
	{
		while (auto f = jabs.mJabs.dequeue()) f();
		std::lock_guard<std::mutex> lock(m);
		jabs.Used = false;
	}
};

class Player
{
public:
	int HP;
};


void Test()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);							// 시스템 정보를 얻어옴
	int cpu = info.dwNumberOfProcessors * 2 + 1;	// cpu 코어 개수

	Player p1;
	Player p2;
	Player p3;
	Player p4;

	p1.HP = 0;
	p2.HP = 0;

	vector<thread*> Th;

	JabPool jp;
	


	for (int i = 0; i < cpu; i++)
	{
		Th.push_back(new thread([&jp] { jp.Run(); }));
	}

	for (int i = 0; i < 10000; i++) jp.Add(&p1, [&] { p1.HP++; });
	for (int i = 0; i < 10000; i++) jp.Add(&p2, [&] { p1.HP++; });
	for (int i = 0; i < 10000; i++) jp.Add(&p2, [&] { p1.HP++; });

	for (auto i : Th) i->join();

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;

	std::cout << "Test() 함수를 수행하는 걸린 시간(초) : " << sec.count() << " seconds" << std::endl;
}