#include "stdafx.h"
#include "ThreadPool.h"
#include "WorkerThread.h"
#include "Job.h"

CWorkerThread::CWorkerThread()
{
	m_job = NULL;
	m_jobData = NULL;
	m_threadPool = NULL;
}

CWorkerThread::~CWorkerThread()
{
}

void CWorkerThread::Run()
{
	for (;;)
	{
		// �ȴ�����
		while (m_job == NULL && !GetExit())
		{
			//printf("thread [%d] wait for job\n", GetThreadID());
			m_jobCond.Wait();
		}

		// �������� �� �߳��˳�
		if (NULL == m_job)
		{
			if (GetExit())
				break;
			else
			{
				//printf("job illege, skip this job...\n");
				continue;
			}
		}

		//����������
		//printf("thread [%d] accept job [%d]\n", GetThreadID(), m_job->GetJobNo());
		m_job->Run(m_jobData);

		if (m_job)
		{
			delete m_job;
			m_job = NULL;
		}
		m_jobData = NULL;
		m_threadPool->MoveToIdleList(this);

		//�������Ŀ����߳�
// 		if (m_threadPool->m_idleList.size() > m_threadPool->GetAvailHighNum())
// 		{
// 			int num = m_threadPool->m_idleList.size() - m_threadPool->GetInitNum();
// 			m_threadPool->DeleteIdleThread(num);
// 		}
	}

	printf("thread [%d] exit\n", GetThreadID());
}

void CWorkerThread::SetJob(CJob* job, void* jobData)
{
	if (!job) return;

	m_job = job;
	m_jobData = jobData;
	job->SetWorkThread(this);
	m_jobCond.Signal();
	//printf("job [%d] add to thread pool\n", m_job->GetJobNo());
}

void CWorkerThread::SetThreadPool(CThreadPool* pool)
{
	if (!pool) return;

	m_threadPool = pool;
}

void CWorkerThread::Terminate()
{
	//�����߳�ǰ���Ƚ�������
	SetExit(TRUE);
	if (m_job)
	{
		delete m_job;
		m_job = NULL;
	}
	m_jobData = NULL;
	//printf("thread [%d] ready to exit\n", GetThreadID());
	m_jobCond.Signal();
	HANDLE hThread = GetThreadHandle();
	if (hThread)
	{
		WaitForSingleObject(GetThreadHandle(), INFINITE);
		CloseHandle(GetThreadHandle());
	}
}