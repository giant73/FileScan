#pragma once

class CJob;
class CThreadPool;

class CThreadManage
{

private:

	//线程池指针
	CThreadPool* 		m_pool;
	//初始创建线程数目
	int			m_numOfThread;

public:

	CThreadManage();
	CThreadManage(int num);
	virtual ~CThreadManage();

	void Run(CJob* job, void* jobData);
	void TerminateAll(void);
	int GetBusyThreadNum();
};
