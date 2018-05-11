#include "stdafx.h"
#include "assert.h"
#include "stdlib.h"
#include <algorithm>
#include "ThreadPool.h"
#include "WorkerThread.h"
#include "Job.h"

CThreadPool::CThreadPool() :m_shutDown(FALSE)
{
	m_maxNum = CThreadPool::THREAD_MAX_NUM;
	m_availLow = CThreadPool::THREAD_AVAIL_LOW;
	m_initNum = m_availNum = CThreadPool::THREAD_INIT_NUM;
	m_availHigh = CThreadPool::THREAD_AVAIL_HIGH;

	//初始化线程数目不超过最大线程数目
	assert(m_initNum > 0 && m_initNum <= m_maxNum);

	//保证初始化的线程数目在最小和最大的空闲线程数目之间则合理
	assert(m_initNum >= m_availLow && m_initNum <= m_availHigh);

	m_threadList.clear();
	m_busyList.clear();
	m_idleList.clear();
	for (unsigned int i = 0; i < m_initNum; ++i)
	{
		CWorkerThread* thr = new CWorkerThread();
		assert(thr);
		thr->SetThreadPool(this);
		AppendToIdleList(thr);
		thr->Start();
	}
}

CThreadPool::CThreadPool(int initNum) :m_shutDown(FALSE)
{
	m_maxNum = CThreadPool::THREAD_MAX_NUM;
	m_availLow = CThreadPool::THREAD_AVAIL_LOW;
	m_initNum = m_availNum = initNum;
	m_availHigh = CThreadPool::THREAD_AVAIL_HIGH;

	//初始化线程数目不超过最大线程数目
	assert(initNum > 0 && (unsigned int)initNum <= m_maxNum);

	//保证初始化的线程数目在最小和最大的空闲线程数目之间则合理
	assert((unsigned int)initNum >= m_availLow && (unsigned int)initNum <= m_availHigh);

	m_threadList.clear();
	m_busyList.clear();
	m_idleList.clear();
	for (unsigned int i = 0; i < (unsigned int)initNum; ++i)
	{
		CWorkerThread* thr = new CWorkerThread();
		assert(thr);
		thr->SetThreadPool(this);
		AppendToIdleList(thr);
		thr->Start();
	}
}

CThreadPool::~CThreadPool()
{
	m_threadList.clear();
	m_busyList.clear();
	m_idleList.clear();
}

void CThreadPool::TerminateAll()
{
	m_shutDown = TRUE;
	for (unsigned int i = 0; i < m_threadList.size(); ++i)
	{
		CWorkerThread* thr = m_threadList[i];
		if (!thr) continue;
		thr->Terminate();
		delete thr; 
		thr = NULL;
	}
	return;
}

CWorkerThread* CThreadPool::GetIdleThread(void)
{
	if (m_shutDown) 
		return NULL;

	while (m_idleList.size() == 0)
	{
		//printf("no idle thread, wait pls...\n");
		m_idleCond.Wait();
	}

	m_idleMutex.Lock();
	if (m_idleList.size() > 0)
	{
		CWorkerThread* thr = (CWorkerThread*)m_idleList.front();
		if (!thr)
		{
			m_idleMutex.Unlock();
			return NULL;
		}
		//printf("get idle thread [%d]\n", thr->GetThreadID());
		m_idleMutex.Unlock();
		if (thr->GetThreadState() == THREAD_IDLE)
			return thr;
	}
	m_idleMutex.Unlock();

	return NULL;
}

void CThreadPool::AppendToIdleList(CWorkerThread* jobThread)
{
	if (m_shutDown) return;

	if (!jobThread) return;

	m_idleMutex.Lock();
	jobThread->SetThreadState(THREAD_IDLE);
	m_idleList.push_back(jobThread);
	m_threadList.push_back(jobThread);
	m_idleMutex.Unlock();
}

void CThreadPool::MoveToBusyList(CWorkerThread* idleThread)
{
	if (m_shutDown) return;

	if (!idleThread) return;

	m_busyMutex.Lock();
	idleThread->SetThreadState(THREAD_BUSY);
	m_busyList.push_back(idleThread);
	m_availNum--;
	m_busyMutex.Unlock();

	m_idleMutex.Lock();
	vector<CWorkerThread*>::iterator iter;
	iter = find(m_idleList.begin(), m_idleList.end(), idleThread);
	if (iter != m_idleList.end())
		m_idleList.erase(iter);
	m_idleMutex.Unlock();
}

void CThreadPool::MoveToIdleList(CWorkerThread* busyThread)
{
	if (m_shutDown)	return;

	if (!busyThread) return;

	m_idleMutex.Lock();
	busyThread->SetThreadState(THREAD_IDLE);
	m_idleList.push_back(busyThread);
	m_availNum++;
	m_idleMutex.Unlock();

	m_busyMutex.Lock();
	vector<CWorkerThread*>::iterator iter;
	iter = find(m_busyList.begin(), m_busyList.end(), busyThread);
	if (iter != m_busyList.end())
		m_busyList.erase(iter);
	m_busyMutex.Unlock();

	m_idleCond.Signal();
	m_maxNumCond.Signal();
}

void CThreadPool::CreateIdleThread(int num)
{
	if (m_shutDown)	return;

	if (num <= 0) return;

	//printf("add new %d threads\n", num);
	for (int i = 0; i < num; ++i)
	{
		CWorkerThread* thr = new CWorkerThread();
		if (!thr) continue;
		thr->SetThreadPool(this);
		AppendToIdleList(thr);
		m_varMutex.Lock();
		m_availNum++;
		m_varMutex.Unlock();
		thr->Start();
	}
}

void CThreadPool::DeleteIdleThread(int num)
{
	if (m_shutDown)	return;

	if (num <= 0) return;

	m_idleMutex.Lock();

	//如果空闲线程数目小于允许最大空闲线程数目，则不再删除
	if (m_idleList.size() < m_availHigh)
	{
		//printf("idle thread list size=%d\n", m_idleList.size());
		m_idleMutex.Unlock();
		return;
	}

	//如果空闲列表头节点线程无效，则本次删除操作失效
	//printf("delete %d threads\n", num);
	for (int i = 0; i < num; ++i)
	{
		CWorkerThread* thr = NULL;
		if (m_idleList.size() > 0)
			thr = (CWorkerThread*)m_idleList.front();

		if (!thr) continue;
		if (THREAD_IDLE != thr->GetThreadState()) continue;
		vector<CWorkerThread*>::iterator  iter;
		iter = find(m_idleList.begin(), m_idleList.end(), thr);
		if (iter != m_idleList.end())
		{
			//printf("delete thread [%d]\n", (*iter)->GetThreadID());
			DestroyIdleThread(*iter);
			m_idleList.erase(iter);
		}

		m_availNum--;
	}
	//printf("after delete, idle threads num:%d, idle list size=%d\n", m_availNum, m_idleList.size());
	m_idleMutex.Unlock();
}

void CThreadPool::DestroyIdleThread(CWorkerThread* idleThread)
{
	if (m_shutDown)	return;

	if (!idleThread) return;

	if (THREAD_IDLE != idleThread->GetThreadState()) return;

	vector<CWorkerThread*>::iterator  iter;
	iter = find(m_threadList.begin(), m_threadList.end(), idleThread);
	if (iter != m_threadList.end())
	{
		CWorkerThread *delThread = *iter;
		if (!delThread) return;
		delThread->Terminate();
		delete delThread; delThread = NULL;
		m_threadList.erase(iter);
	}
}

void CThreadPool::Run(CJob* job, void* JobData)
{
	if (m_shutDown)	return;

	assert(job != NULL);

	//线程池已经达到最大并发数目，则等待
	if (GetBusyNum() >= m_maxNum)
	{
		//printf("busy threads reach max permit num:%d\n", m_maxNum);
		m_maxNumCond.Wait();
	}

	//补充空闲线程(保持空闲线程数目为m_initNum)
	if (m_idleList.size() < m_availLow)
	{
		if (GetAllNum() + m_initNum - m_idleList.size() < m_maxNum)
			CreateIdleThread(m_initNum - m_idleList.size());
		else
			CreateIdleThread(m_maxNum - GetAllNum());
	}

	CWorkerThread* idleThr = GetIdleThread();
	if (idleThr != NULL)
	{
		MoveToBusyList(idleThr);
		idleThr->SetThreadPool(this);
		job->SetWorkThread(idleThr);
		idleThr->SetJob(job, JobData);
		//printf("job [%d] is bind to thread [%d]\n", job->GetJobNo(), idleThr->GetThreadID());
	}
}


