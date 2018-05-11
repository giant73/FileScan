#include "stdafx.h"
#include "ThreadMutex.h"

CThreadMutex::CThreadMutex()
{
	::InitializeCriticalSection(&m_cs);
}

CThreadMutex::~CThreadMutex()
{
	::DeleteCriticalSection(&m_cs);
}

BOOL CThreadMutex::Lock()
{
	::EnterCriticalSection(&m_cs);
	return TRUE;
}

BOOL CThreadMutex::Unlock()
{
	::LeaveCriticalSection(&m_cs);
	return TRUE;
}

BOOL CThreadMutex::TryLock()
{
	BOOL ret = TryEnterCriticalSection(&m_cs);
	return ret;
}