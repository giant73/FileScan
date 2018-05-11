#include "stdafx.h"
#include "Thread.h"

CThread::CThread() :
	m_isExit(FALSE),
	m_threadHandle(0),
	m_threadID(0),
	m_threadName(NULL),
	m_threadState(THREAD_INIT)
{

}

CThread::~CThread()
{
	if (NULL != m_threadName)
	{
		free(m_threadName);
		m_threadName = NULL;
	}
}

BOOL CThread::Terminate()
{
	if (m_isExit)
	{
		_endthreadex(0);
		if (m_threadHandle)
			CloseHandle(m_threadHandle);
		return TRUE;
	}
	return FALSE;
}

unsigned __stdcall CThread::ThreadFunction(void* pArg)
{
	CThread* pThread = (CThread*)pArg;
	pThread->Run();
	return 0;
}

BOOL CThread::Start()
{
	unsigned int threadID = 0;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunction, this, 0, &threadID);
	if (hThread == 0)
	{
		printf("create thread fail, errno=%d, doserrno=%d\n", errno, _doserrno);
		return FALSE;
	}
	this->m_threadID = threadID;
	this->m_threadHandle = hThread;
	//printf("new thread [%d]\n", threadID);
	return TRUE;
}


void CThread::SetThreadName(char* thrName)
{
	if (NULL != m_threadName)
	{
		free(m_threadName);
		m_threadName = NULL;
	}

	if (NULL != thrName)
	{
		int mallocSize = strlen(thrName) + 1;
		m_threadName = (char*)malloc(mallocSize);
		memset(m_threadName, 0, mallocSize);
		strncpy_s(m_threadName, mallocSize, thrName, strlen(thrName));
	}
}
