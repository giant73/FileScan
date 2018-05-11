
#include "stdafx.h"
#include "stdlib.h"
#include "Job.h"

CJob::CJob()
	:m_pWorkThread(NULL),
	m_jobNo(0),
	m_jobName(NULL)
{
}

CJob::~CJob()
{
	if (NULL != m_jobName)
	{
		free(m_jobName);
		m_jobName = NULL;
	}

	m_pWorkThread = NULL;
}

void CJob::SetJobName(const char* jobName)
{
	if (NULL != m_jobName)
	{
		free(m_jobName);
		m_jobName = NULL;
	}

	if (NULL != jobName)
	{
		int mallocSize = strlen(jobName) + 1;
		m_jobName = (char*)malloc(mallocSize);
		memset(m_jobName, 0, mallocSize);
		strncpy_s(m_jobName, mallocSize, jobName, strlen(jobName));
	}
}