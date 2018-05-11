#include "stdafx.h"
#include "ThreadCondition.h"

CThreadCondition::CThreadCondition()
{
	m_hCond = ::CreateEvent(NULL, TRUE, FALSE, NULL);
}

CThreadCondition::~CThreadCondition()
{
	if (NULL != m_hCond)
		::CloseHandle(m_hCond);
}

void CThreadCondition::Wait()
{
	WaitForSingleObject(m_hCond, INFINITE);
	ResetEvent(m_hCond);
}

DWORD CThreadCondition::Wait(DWORD time)
{
	DWORD result =  WaitForSingleObject(m_hCond, time);
	ResetEvent(m_hCond);
	return result;
}

void CThreadCondition::Signal()
{
	SetEvent(m_hCond);
}

