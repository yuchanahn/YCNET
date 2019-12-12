#pragma once


class ThreadData
{
	int strandID;
	vector<queue<function<void()>>>& c;
	vector<queue<function<void()>>>& f;
	vector<bool>& cj;
public:
	ThreadData(vector<queue<function<void()>>>& c_, vector<queue<function<void()>>>& f_, vector<bool>& cj_) : 
		c(c_), f(f_), cj(cj_), strandID(-1) {}

	bool GetNewStrand();

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
	std::mutex mt;
	std::condition_variable c;
private:

	vector<bool> isCurrentJob;
	vector<queue<function<void()>>> mCurrentJobs;
	vector<queue<function<void()>>> mFutureJobs;

	bool Stop;
	
	void Init();

public:
	JobManager() : Stop(false) { Init(); }

	void StopThread();
	void add_Job(int id, function<void()> f);
	int get_strand();
	void run();
};



class Strand
{
	int id;
	JobManager& jm;

	void Init();
public:
	Strand(JobManager& jm_) : jm(jm_)
	{
		Init();
	}
	void push(function<void()> job);
};