#include "stdafx.h"
#include "ThreadManage.h"
#include "ThreadPool.h"
#include "Job.h"

CThreadManage::CThreadManage()
{
	m_numOfThread = 10;
	m_pool = new CThreadPool(m_numOfThread);
}

CThreadManage::CThreadManage(int num)
{
	m_numOfThread = num;
	m_pool = new CThreadPool(m_numOfThread);
}

CThreadManage::~CThreadManage()
{
	if (NULL != m_pool)
	{
		delete m_pool;
		m_pool = NULL;
	}
}

void CThreadManage::Run(CJob* job, void* jobData)
{
	m_pool->Run(job, jobData);
}

void CThreadManage::TerminateAll(void)
{
	m_pool->TerminateAll();
}

int CThreadManage::GetBusyThreadNum()
{
	return m_pool->GetBusyNum();
}