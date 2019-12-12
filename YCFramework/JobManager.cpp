#include "pch.h"
#include "JobManager.h"

bool ThreadData::GetNewStrand()
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


void JobManager::Init()
{
}

void JobManager::add_Job(int id, function<void()> f)
{
	{
		std::unique_lock<std::mutex> lock(mt);

		if (Stop) throw std::runtime_error("enqueue on stopped ThreadPool");

		if (isCurrentJob[id])	mCurrentJobs[id].push(std::move(f));
		else					mFutureJobs[id].push(std::move(f));
	}
	c.notify_one();
}

int JobManager::get_strand()
{
	std::lock_guard<std::mutex> lock(mt);
	static int strand = 0;
	mCurrentJobs.push_back(queue <function<void()>>());
	mFutureJobs.push_back(queue <function<void()>>());
	isCurrentJob.push_back(false);
	return strand++;
}

void JobManager::run()
{
	ThreadData self(mCurrentJobs, mFutureJobs, isCurrentJob);
	while (true)
	{
		function<void()> task;

		{
			std::unique_lock<std::mutex> lock(mt);
			c.wait(lock, [&] { return Stop || (self.IsTasking ? true : self.GetNewStrand()); });
			if (Stop) return;
			task = std::move((self.Strand)->front());
			(self.Strand)->pop();
			lock.unlock();
			task();
		}
	}
}


void JobManager::StopThread()
{
	std::lock_guard<std::mutex> lock(mt);
	Stop = true;
}

void Strand::Init()
{
	id = jm.get_strand();
}

void Strand::push(function<void()> job)
{
	jm.add_Job(id, job);
}
