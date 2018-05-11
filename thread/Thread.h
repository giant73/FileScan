#pragma once

typedef enum _ThreadState
{
	THREAD_INIT = -1,
	THREAD_RUNNING = 0,
	THREAD_IDLE = 1,
	THREAD_EXIT = 2,
	THREAD_BUSY = 3,
}ThreadState;

class CThread
{

private:

	unsigned int 			m_threadID;
	HANDLE					m_threadHandle;
	char*					m_threadName;
	ThreadState				m_threadState;
	BOOL					m_isExit;

protected:

	static unsigned __stdcall ThreadFunction(void*);

public:

	CThread();
	virtual ~CThread();

	virtual void Run(void) = 0;

	BOOL Terminate(void);
	BOOL Start(void);

	void SetThreadState(ThreadState state) { m_threadState = state; }
	ThreadState GetThreadState(void) { return m_threadState; }

	void SetThreadName(char* thrName);
	char* GetThreadName(void) { return m_threadName; }

	BOOL GetExit() { return m_isExit; }
	void SetExit(BOOL exit) { m_isExit = exit; }

	HANDLE GetThreadHandle() { return m_threadHandle; }
	void   SetThreadHandle(HANDLE hdl) { m_threadHandle = hdl; }

	int GetThreadID(void) { return m_threadID; }
};