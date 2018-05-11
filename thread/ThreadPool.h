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

	//��ȡһ�������߳�
	CWorkerThread* GetIdleThread(void);
	//׷���µĿ����߳�
	void AppendToIdleList(CWorkerThread* jobThread);
	//�����߳�תæµ�߳�
	void MoveToBusyList(CWorkerThread* idleThread);
	//æµ�߳�ת�����߳�
	void MoveToIdleList(CWorkerThread* busyThread);
	//ɾ�������߳�
	void DeleteIdleThread(int num);
	//���ٿ����߳�
	void DestroyIdleThread(CWorkerThread* idleThread);
	//�����µĿ����߳�
	void CreateIdleThread(int num);

public:

	//�ٽ���
	CThreadMutex			m_busyMutex;
	CThreadMutex			m_idleMutex;
	CThreadMutex			m_jobMutex;
	CThreadMutex			m_varMutex;

	//�¼������ȴ�
	CThreadCondition		m_busyCond;
	CThreadCondition		m_idleCond;
	CThreadCondition		m_idleJobCond;
	CThreadCondition		m_maxNumCond;

	//�߳��б�
	vector<CWorkerThread*> m_threadList;
	//æµ�߳��б�
	vector<CWorkerThread*> m_busyList;
	//�����߳��б�
	vector<CWorkerThread*> m_idleList;

	//�̳߳���
	static const int THREAD_MAX_NUM = 30;
	static const int THREAD_INIT_NUM = 10;
	static const int THREAD_AVAIL_LOW = 5;
	static const int THREAD_AVAIL_HIGH = 15;

private:

	//�̳߳�������������߳���Ŀ
	unsigned int m_maxNum;
	//�̳߳�������ڵ���С�����߳���Ŀ
	unsigned int m_availLow;
	//�̳߳�������ڵ��������߳���Ŀ
	unsigned int m_availHigh;
	//��ǰ�̳߳��е�ʵ�ʿ����߳���Ŀ(size of m_idleList)
	unsigned int m_availNum;
	//��ʼ�������߳���Ŀ
	unsigned int m_initNum;

	//�����̳߳�
	BOOL	m_shutDown;
};