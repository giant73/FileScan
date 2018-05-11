#pragma once

class CThreadMutex
{

private:
	CRITICAL_SECTION m_cs;

public:

	CThreadMutex();
	~CThreadMutex();

	BOOL Lock();
	BOOL Unlock();
	BOOL TryLock();
};