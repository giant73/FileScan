#pragma once

#include "ThreadCondition.h"
#include "ThreadMutex.h"

class CWorkerThread;
class CJob;

class CThreadPool
{
	friend class CWorkerThread;

public:

	CThreadPool();
	CThreadPool(int initNum);
	virtual ~CThreadPool();

	void TerminateAll(void);
	void Run(CJob* job, void* jobData);

	void SetMaxNum(int maxNum) { m_maxNum = maxNum; }
	unsigned int  GetMaxNum(void) { return m_maxNum; }
	void SetAvailLowNum(int minNum) { m_availLow = minNum; }
	unsigned int  GetAvailLowNum(void) { return m_availLow; }
	void SetAvailHighNum(int highNum) { m_availHigh = highNum; }
	unsigned int  GetAvailHighNum(void) { return m_availHigh; }
	unsigned int GetActualAvailNum(void) { return m_availNum; }
	unsigned int  GetAllNum(void) { return m_threadList.size(); }
	unsigned int	 GetBusyNum(void) { return m_busyList.size(); }
	void SetInitNum(int initNum) { m_initNum = initNum; }
	unsigned int  GetInitNum(void) { return m_initNum; }

protected:

	//获取一个空闲线程
	CWorkerThread* GetIdleThread(void);
	//追加新的空闲线程
	void AppendToIdleList(CWorkerThread* jobThread);
	//空闲线程转忙碌线程
	void MoveToBusyList(CWorkerThread* idleThread);
	//忙碌线程转空闲线程
	void MoveToIdleList(CWorkerThread* busyThread);
	//删除空闲线程
	void DeleteIdleThread(int num);
	//销毁空闲线程
	void DestroyIdleThread(CWorkerThread* idleThread);
	//创建新的空闲线程
	void CreateIdleThread(int num);

public:

	//临界区
	CThreadMutex			m_busyMutex;
	CThreadMutex			m_idleMutex;
	CThreadMutex			m_jobMutex;
	CThreadMutex			m_varMutex;

	//事件触发等待
	CThreadCondition		m_busyCond;
	CThreadCondition		m_idleCond;
	CThreadCondition		m_idleJobCond;
	CThreadCondition		m_maxNumCond;

	//线程列表
	vector<CWorkerThread*> m_threadList;
	//忙碌线程列表
	vector<CWorkerThread*> m_busyList;
	//空闲线程列表
	vector<CWorkerThread*> m_idleList;

	//线程常量
	static const int THREAD_MAX_NUM = 30;
	static const int THREAD_INIT_NUM = 10;
	static const int THREAD_AVAIL_LOW = 5;
	static const int THREAD_AVAIL_HIGH = 15;

private:

	//线程池允许并发的最大线程数目
	unsigned int m_maxNum;
	//线程池允许存在的最小空闲线程数目
	unsigned int m_availLow;
	//线程池允许存在的最大空闲线程数目
	unsigned int m_availHigh;
	//当前线程池中的实际空闲线程数目(size of m_idleList)
	unsigned int m_availNum;
	//初始创建的线程数目
	unsigned int m_initNum;

	//销毁线程池
	BOOL	m_shutDown;
};