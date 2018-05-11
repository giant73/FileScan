#pragma once

class CThreadCondition
{

private:

	HANDLE m_hCond;

public:

	CThreadCondition();
	~CThreadCondition();

	void Wait();
	void Signal();
	DWORD Wait(DWORD time);
};