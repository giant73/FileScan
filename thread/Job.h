#pragma once

class CThread;
class CJob
{

private:

	uint64		m_jobNo;
	char*		m_jobName;
	CThread*	m_pWorkThread;

public:

	CJob();
	virtual ~CJob();

	uint64	GetJobNo() const { return m_jobNo; }
	void SetJobNo(uint64 JobNo) { m_jobNo = JobNo; }
	char*	GetJobName() const { return m_jobName; }
	void SetJobName(const char* JobName);
	CThread* GetWorkThread() { return m_pWorkThread; }
	void	SetWorkThread(CThread *pWorkThread) { m_pWorkThread = pWorkThread; }
	virtual void Run(void* ptr) = 0;
};