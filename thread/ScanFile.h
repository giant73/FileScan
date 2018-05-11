#pragma once
#include "Job.h"
#include "ThreadManage.h"
#include "ThreadMutex.h"
#include "ThreadCondition.h"

class CScanFile :public CJob
{

public:

	CScanFile(string path):strPath(path){};
	~CScanFile() {}
	void Run(void* jobData);

private:
	CThreadMutex  m_Mutex;
	CThreadCondition m_FinishEvent;
	string strPath;
public:
	uint64 ui64Id;
};