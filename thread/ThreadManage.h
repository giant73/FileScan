#pragma once

class CJob;
class CThreadPool;

class CThreadManage
{

private:

	//�̳߳�ָ��
	CThreadPool* 		m_pool;
	//��ʼ�����߳���Ŀ
	int			m_numOfThread;

public:

	CThreadManage();
	CThreadManage(int num);
	virtual ~CThreadManage();

	void Run(CJob* job, void* jobData);
	void TerminateAll(void);
	int GetBusyThreadNum();
};
