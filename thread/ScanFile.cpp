#include "stdafx.h"
#include "Job.h"
#include "ThreadManage.h"
#include "tool.h"
#include "ScanFile.h"
#include <map>
#include <queue>

#define MAX_SEARCH_PATH 16

map<string, int> mapUserInput; //�û����������·��

struct ChildFolderData
{
	uint64 id;
	string path;
};

queue<ChildFolderData> queSearchPath;  //Ҫ��������Ŀ¼·��

map<uint64, uint64> mapFolderSize; //��¼����һ����Ŀ¼��С

//vector<uint64> vectForderSize; //�����ļ��д�С
vector<string> vectFileInfo;  //���ļ���С
map<uint64, vector<string>> mapFileInfo;

bool GetRootContent[MAX_SEARCH_PATH] = {};

CThreadMutex g_WriteMutex;
CThreadCondition g_ThreadFinishEvent;

void CScanFile::Run(void* jobData)
{
	string path = strPath;// *(string*)jobData;

	uint64 ui64FolderSize = 0;			//��Ŀ¼�����ļ��ܴ�С
	intptr_t  FileHandle = 0;          //�ļ����                          
	struct _finddata_t FileInfo; //�ļ���Ϣ
	string FileName = "";
	vector<string> ChildFolder;					//��ǰĿ¼�µ���Ŀ¼
	vector<string> vectFileSizeInfo;			//��ǰĿ¼Ҫ��ӡ�����ļ���Ϣ
	if ((FileHandle = _findfirst(FileName.assign(path).append("\\*").c_str(), &FileInfo)) != -1)
	{
		do
		{
			uint64 ui64FileSize = 0;
			//�����Ŀ¼,����ɨ�����;�������,�����б�  
			if ((FileInfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(FileInfo.name, ".") != 0 && strcmp(FileInfo.name, "..") != 0)
				{
					string strChildFolderPath = FileName.assign(path).append("\\").append(FileInfo.name);
					ChildFolder.push_back(strChildFolderPath);
				}
			}
			else
			{
				string strCurrentFileInfo = FileName.assign(path).append("\\").append(FileInfo.name).append("  ").append(_w2s(FormatSize(FileInfo.size).c_str()));
				vectFileSizeInfo.push_back(strCurrentFileInfo);
				ui64FileSize = FileInfo.size;
			}
			ui64FolderSize += ui64FileSize;
		} while (0 == _findnext(FileHandle, &FileInfo));

		_findclose(FileHandle);
	}
	else
	{
		cout << "·�� \"" << path << "\" δ�ҵ���" << endl;
		return;
	}

	g_WriteMutex.Lock();

	if (!GetRootContent[ui64Id])
	{
		GetRootContent[ui64Id] = true;

		//����һ����Ŀ¼id
		for (int i = 0; i < ChildFolder.size(); ++i)
		{
			uint64 id = ui64Id << 24 + i + 1; //ǰ8λΪ����id,��24λΪ�ļ���id
			ChildFolderData data = { id, ChildFolder[i] };
			queSearchPath.push(data);
			mapFolderSize.insert(pair<uint64, uint64>(id, 0));
		}

		map<uint64, vector<string>>::iterator iterFileInfo;
		iterFileInfo = mapFileInfo.find(ui64Id);
		if (iterFileInfo != mapFileInfo.end())
		{
			iterFileInfo->second = vectFileInfo;
		}
		vectFileInfo.insert(vectFileInfo.end(), vectFileSizeInfo.begin(), vectFileSizeInfo.end());
	}
	else
	{
		for (int i = 0; i < ChildFolder.size(); ++i)
		{
			ChildFolderData data = { ui64Id, ChildFolder[i] };
			queSearchPath.push(data);
		}
	}

	map<uint64, uint64>::iterator iterFolderSize;
	iterFolderSize = mapFolderSize.find(ui64Id);
	if (iterFolderSize != mapFolderSize.end())
	{
		iterFolderSize->second += ui64FolderSize;
	}
	//vectForderSize.push_back(ui64FolderSize);
	//vectFileInfo.insert(vectFileInfo.end(), vectFileSizeInfo.begin(), vectFileSizeInfo.end());
	g_WriteMutex.Unlock();

	//֪ͨ���̱߳���ɨ�����
	g_ThreadFinishEvent.Signal();
}

 int _tmain(int argc, _TCHAR* argv[])
 {
	 // ������Ļ��������С(��λ:�ַ���)
	 HANDLE hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	 COORD BuffSize;
	 BuffSize.X = 100;
	 BuffSize.Y = 3000;
	 SetConsoleScreenBufferSize(hConsoleHandle, BuffSize);

	 CThreadManage* pThreadManage = new CThreadManage(10);
	 double  dStartTime = 0, dFinishTime = 0;

 INPUT:
	 {	
		 uint64 nWorkNum = 1;
		 cout << "��ÿ������һ��·�������롰start����ʼ���������롰end����������" << endl;
		 string strCurrentInput; //��ǰ�����ַ���
		 while (getline(cin, strCurrentInput))
		 {
			 if (strCurrentInput == "end")
			 {
				 goto END;
			 }
			 else if (strCurrentInput == "start")
			 {
				 if (!mapUserInput.size())
				 {
					 cout << "����·��Ϊ�գ����������룡" << endl;
					 continue;
				 }
				 else
				 {
					 cout << "��ʼɨ��..." << endl;				
					 break;
				 }
			 }
			 //queSearchPath.push(strCurrentInput);	
			 mapUserInput.insert(pair<string, int>(strCurrentInput, nWorkNum));
			 if (nWorkNum >= MAX_SEARCH_PATH)
			 {
				 cout << "��ʼɨ��..." << endl;
				 break;
			 }
			 else
				 ++nWorkNum;
		 }
	 }
#ifdef SINGLE_THREAD
	 StartScan(queSearchPath);
#else

	 dStartTime = clock();//ȡ��ʼʱ��

	 for (auto path_map: mapUserInput)
	 {
		 string path = FormatDirPath(path_map.first);
		 CScanFile* job = new CScanFile(path);
		 job->ui64Id = path_map.second;
		 job->SetJobNo(path_map.second);
		 job->SetJobName(path.c_str());
		 string *data = &path;
		 pThreadManage->Run(job, data);
	 }

	 while (true)
	 {
		 for (; queSearchPath.size();)
		 {
			 g_WriteMutex.Lock();
			 string path = FormatDirPath(queSearchPath.front().path);
			 uint64 id  = queSearchPath.front().id;
			 queSearchPath.pop();
			 g_WriteMutex.Unlock();
			 CScanFile* job = new CScanFile(path);
			 job->ui64Id = id;
			 job->SetJobNo(id);
			 job->SetJobName(path.c_str());
			 string *data = &path;
			 pThreadManage->Run(job, data);
		 }
		 DWORD result = g_ThreadFinishEvent.Wait(100);
		 if (result == WAIT_TIMEOUT && !queSearchPath.size() && !pThreadManage->GetBusyThreadNum())//��ʱ���Ҵ�ɨ�����Ϊ�գ�æµ�߳�ҲΪ�գ�����Ϊɨ�����
		 	break;
	 }
	 dFinishTime = clock();//ȡ��ʼʱ��
#endif // SINGLE_THREAD

	 //������
	 {
		 uint64 d = 0;
		 for (auto n : mapUserInput)
		 {
			 cout << "·��" << "\"" << n.first << "\":" << "�ܴ�С" << d << endl;
		 }
		 wcout << FormatSize(d) << endl;
		 double dd = (dFinishTime - dStartTime) / CLOCKS_PER_SEC;
		 cout << dd << endl;
	 }

	 //�ص�����״̬
	 //vectForderSize.clear();
	 vectFileInfo.clear();
	 goto INPUT;

 END:
	 {
		 pThreadManage->TerminateAll();
		 delete pThreadManage;
		 pThreadManage = NULL;
	 }
	return 0;
 }







