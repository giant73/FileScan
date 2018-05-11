#pragma once

#include "Thread.h"
#include "ThreadCondition.h"
#include "ThreadMutex.h"

class CJob;
class CThreadPool;

class CWorkerThread : public CThread
{

private:

	CThreadPool* 			m_threadPool;
	CJob*					m_job;
	void*					m_jobData;

public:

	CThreadCondition		m_jobCond;

public:

	CWorkerThread();
	virtual ~CWorkerThread();
	void	Run();
	void	Terminate();
	void	SetJob(CJob* job, void* jobData);
	CJob*	GetJob(void) { return m_job; }
	void	SetThreadPool(CThreadPool* thrPool);
	CThreadPool*	GetThreadPool(void) { return m_threadPool; }

};