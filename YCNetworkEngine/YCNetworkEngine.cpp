#include "pch.h"
#include "ThreadPool.h"

class Ptest {
public:
	PROP(int, Hp,
		{ return _Hp; },
		{ _Hp = value; }
	);
};


class ThreadData
{
	int strandID;
	vector<queue<function<void()>>>& c;
	vector<queue<function<void()>>>& f;
	vector<bool>& cj;
public:
	ThreadData(vector<queue<function<void()>>>& c_, vector<queue<function<void()>>>& f_, vector<bool>& cj_) : c(c_), f(f_), cj(cj_), strandID(-1)
	{}

	bool GetNewStrand()
	{
		for (int i = 0; i < f.size(); i++)
		{
			if (!f[i].empty())
			{
				c[i] = std::move(f[i]);
				if (strandID != -1) cj[strandID] = false;
				strandID = i;
				cj[i] = true;
				return true;
			}
		}
		return false;
	}

	PROP_G(bool, IsTasking, {
		if (strandID != -1) {
			if (c[strandID].empty())
			{
				cj[strandID] = false;
				strandID = -1;
			}
		}
		return strandID != -1;
		});
	PROP_G(queue<function<void()>>*, Strand, { return  &c[strandID]; });
};


class JobManager
{

private:
	mutex mt;
	std::condition_variable c;
private:

	vector<bool> isCurrentJob;
	vector<queue<function<void()>>> mCurrentJobs;
	vector<queue<function<void()>>> mFutureJobs;

	bool Stop;

public:
	JobManager() : Stop(false) 
	{
		for (int i = 0; i < 5; i++)
		{
			mCurrentJobs.push_back(queue <function<void()>>());
			mFutureJobs.push_back(queue <function<void()>>());
			isCurrentJob.push_back(false);
		}
	}
	void add_Job(int id, function<void()> f)
	{
		{
			std::unique_lock<std::mutex> lock(mt);

			if (Stop) throw std::runtime_error("enqueue on stopped ThreadPool");
			
			if (isCurrentJob[id])	mCurrentJobs[id].push(std::move(f));
			else					mFutureJobs[id].push(std::move(f));
		}
		c.notify_one();
	}
	void run()
	{
		ThreadData self(mCurrentJobs, mFutureJobs, isCurrentJob);
		while (true)
		{
			function<void()> task;

			{
				std::unique_lock<mutex> lock(mt);
				c.wait(lock, [&] { return Stop || (self.IsTasking ? true : self.GetNewStrand()); });
				if (Stop) return;
				task = std::move((self.Strand)->front());
				(self.Strand)->pop();
				lock.unlock();
				task();
			}
		}
	}


};





int main()
{
	JobManager jd;

	vector<thread> t;
	ThreadPool tp(4);

	for (int i = 0; i < 4; i++)
		t.push_back(thread([&] { jd.run(); }));


 	atomic<int> n1 = 1;
	atomic<int> n2 = 1;
	atomic<int> n3 = 1;
	atomic<int> n4 = 1;
	atomic<int> n5 = 1;


	atomic<int> n11 = 1;
	atomic<int> n12 = 1;
	atomic<int> n13 = 1;
	atomic<int> n14 = 1;
	atomic<int> n15 = 1;

	int n6 = 1;
	int n[54] = { 0 };



	for (int i = 0; i < 10000; i++)
	{
		jd.add_Job(0, [&] { n1++; if (n1 == 10000) printf("1\n"); });
		jd.add_Job(1, [&] { n2++; if (n2 == 10000) printf("2\n"); });
		jd.add_Job(2, [&] { n3++; if (n3 == 10000) printf("3\n"); });
		jd.add_Job(3, [&] { n4++; if (n4 == 10000) printf("4\n"); });
		jd.add_Job(4, [&] { n5++; if (n5 == 10000) printf("5\n"); });
	}

	for (auto& i : t) i.join();
	//Test();



	while (1);

	//char buffer[1024] = { 0 };
	//char cBuf[1024] = { 0 };

	//std::cin >> buffer;

	//std::copy(buffer, buffer + strlen(buffer), cBuf);

	//printf("%s[len : %lld]\n", cBuf, strlen(buffer));
}