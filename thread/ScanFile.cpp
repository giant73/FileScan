#include "stdafx.h"
#include "Job.h"
#include "ThreadManage.h"
#include "tool.h"
#include "ScanFile.h"
#include <map>
#include <queue>

#define MAX_SEARCH_PATH 16
struct ChildFolderData
{
	uint64 id;
	string path;
};

#ifdef MODE_1
map<string, uint64> mapUserInput; //�û����������·��:<·��������id>

queue<ChildFolderData> queSearchPath;  //Ҫ��������Ŀ¼·��

map<uint64, uint64> mapFolderSize;	//��¼����һ����Ŀ¼��С:<�ļ���id���ļ��д�С>
map<uint64, string> mapFolderString; //��¼����һ����Ŀ¼·����Ϣ:<�ļ���id���ļ���·��>

//vector<uint64> vectForderSize; //�����ļ��д�С
//vector<string> vectFileInfo;  //���ļ���С

map<uint64, vector<string>> mapFileInfo;

bool GetRootContent[MAX_SEARCH_PATH] = {false};

CThreadMutex g_WriteMutex;
CThreadCondition g_ThreadFinishEvent;

void CScanFile::Run(void* jobData)
{
	string path = strPath;

	uint64 ui64FolderSize = 0;			//��Ŀ¼�����ļ��ܴ�С
	intptr_t  FileHandle = 0;          //�ļ����                          
	struct _finddatai64_t FileInfo;		//�ļ���Ϣ
	string FileName = "";
	vector<string> ChildFolder;					//��ǰĿ¼�µ���Ŀ¼
	vector<string> vectFileSizeInfo;			//��ǰĿ¼Ҫ��ӡ�����ļ���Ϣ
	if ((FileHandle = _findfirsti64(FileName.assign(path).append("\\*").c_str(), &FileInfo)) != -1)
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
				string strCurrentFileInfo = FileName.assign(path).append("\\").append(FileInfo.name).append("  ").append(_w2s(FormatSize(FileInfo.size).c_str())).append("\n");
				vectFileSizeInfo.push_back(strCurrentFileInfo);
				ui64FileSize = FileInfo.size;
			}
			ui64FolderSize += ui64FileSize;
		} while (0 == _findnext64(FileHandle, &FileInfo));

		_findclose(FileHandle);
	}
	else
	{
		//cout << "·�� \"" << path << "\" δ�ҵ���" << endl;
		return;
	}

	g_WriteMutex.Lock();

	//����id�ж��Ǹ�Ŀ¼������56λΪ0�����Ǹ�Ŀ¼
	if ( !((ui64Id) & 0x00ffffff) && !GetRootContent[(ui64Id>>56)])
	{
		GetRootContent[(ui64Id >> 56)] = true;
		mapFolderSize.insert(pair<uint64, uint64>(ui64Id, 0));
		//����һ����Ŀ¼id
		for (int i = 0; i < ChildFolder.size(); ++i)
		{
			uint64 id = ui64Id + i + 1; //ǰ8λΪ����id,��56λΪ�ļ���id
			ChildFolderData data = { id, ChildFolder[i] };
			queSearchPath.push(data);
			mapFolderSize.insert(pair<uint64, uint64>(id, 0));
			mapFolderString.insert(pair<uint64, string>(id, ChildFolder[i]));
		}

		mapFileInfo.insert(pair<uint64, vector<string>>(ui64Id, vectFileSizeInfo));
// 		map<uint64, vector<string>>::iterator iterFileInfo;
// 		iterFileInfo = mapFileInfo.find(ui64Id);
// 		if (iterFileInfo != mapFileInfo.end())
// 		{
// 			iterFileInfo->second = vectFileSizeInfo;
// 		}
		//vectFileInfo.insert(vectFileInfo.end(), vectFileSizeInfo.begin(), vectFileSizeInfo.end());
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
	 BuffSize.X = 300;
	 BuffSize.Y = 6000;
	 SetConsoleScreenBufferSize(hConsoleHandle, BuffSize);

	 DWORD nMode;
	 HANDLE hConsoleHandle2 = GetStdHandle(STD_INPUT_HANDLE);
	 GetConsoleMode(hConsoleHandle2, &nMode);
	 SetConsoleMode(hConsoleHandle2, nMode & ~ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT);

	 CThreadManage* pThreadManage = new CThreadManage(10);
	 double  dStartTime = 0, dFinishTime = 0;

 INPUT:
	 {	
		 uint64 nWorkNum = 0;

		 SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
		 cout << "��ÿ������һ��·�������롰start����ʼ���������롰end����������" << endl;
		 SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

		 string strCurrentInput; //��ǰ�����ַ���
		 while (getline(cin, strCurrentInput))
		 {
			 if (!Trim(strCurrentInput)) //ȥ�������ַ�
				 continue;
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
			 mapUserInput.insert(pair<string, uint64>(strCurrentInput, (nWorkNum<<56)));
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
		 DWORD result = g_ThreadFinishEvent.Wait(200);
		 if (result == WAIT_TIMEOUT && !queSearchPath.size() && !pThreadManage->GetBusyThreadNum())//��ʱ���Ҵ�ɨ�����Ϊ�գ�æµ�߳�ҲΪ�գ�����Ϊɨ�����
		 	break;
	 }
	 dFinishTime = clock();//ȡ��ʼʱ��
#endif // SINGLE_THREAD

	 //������
	 {
		 //����ʹ�ӡ���Բ𿪳ɶ���߳�
		 for (auto path : mapUserInput)
		 {
			 uint64 TotalSize = 0;
			 string strShowFolder = "";
			 string strShowFile = "";
			 for (auto size : mapFolderSize)
			 {
				 if ((size.first>>56) == (path.second>>56)) //��8λidһ�£�˵����Ŀ¼�����ڸø�Ŀ¼
				 {
					 //�����ܴ�С
					 TotalSize += size.second;

					 //���������Ϣ
					 map<uint64, string>::iterator iterFolderInfo;
					 iterFolderInfo = mapFolderString.find(size.first);
					 if (iterFolderInfo != mapFolderString.end())
					 {
						 wstring FolderSize = FormatSize(size.second);
						 strShowFolder.append(iterFolderInfo->second).append("	").append(_w2s(FolderSize)).append("\n");
					 }
				 }
			 }

			 map<uint64, vector<string>>::iterator iterFileInfo;
			 iterFileInfo = mapFileInfo.find(path.second);
			 if (iterFileInfo != mapFileInfo.end())
			 {
				 for (auto file : iterFileInfo->second)
				 {
					 strShowFile.append(file);
				 }
			 }

			 string strShowRoot = "·��";
			 strShowRoot = strShowRoot.append("\"").append(path.first.c_str()).append("\"").append("�ܴ�С:").append(_w2s(FormatSize(TotalSize))).append("\n");

			 cout << strShowRoot;
			 cout << strShowFolder;
			 cout << strShowFile;
			 cout << "--------------------------------------------------" << endl;
		 }
		 double time = (dFinishTime - dStartTime) / CLOCKS_PER_SEC;
		 cout << "����ɨ����ʱ��"<<time<<"seconds" << endl;
		 cout << "*******************************************************" << endl;
	 }

	 //�ص�����״̬
	 //vectForderSize.clear();
	 mapFolderSize.clear();
	 mapFolderString.clear();
	 mapUserInput.clear();
	 mapFileInfo.clear();
	 memset(GetRootContent, false, MAX_SEARCH_PATH); 
	 goto INPUT;

 END:
	 {
		 pThreadManage->TerminateAll();
		 delete pThreadManage;
		 pThreadManage = NULL;
	 }
	return 0;
 }

#else

CThreadMutex g_WriteMutex;
CThreadCondition g_ThreadFinishEvent;

void CScanFile::Run(void* jobData)
{
	bool bFirstGetIn = true;

	vector<string>* vectFileSizeInfo = new vector<string>;			//��ǰĿ¼Ҫ��ӡ�����ļ���Ϣ

	map<uint64, uint64>* mapFolderSize = new map<uint64, uint64>;	//��¼����һ����Ŀ¼��С:<�ļ���id���ļ��д�С>
	map<uint64, vector<string>>* mapFileInfo = new map<uint64, vector<string>>;
	map<uint64, string>* mapFolderString = new map<uint64, string>; //��¼����һ����Ŀ¼·����Ϣ:<�ļ���id���ļ���·��>

	queue<ChildFolderData>* queChildFolder = new queue<ChildFolderData>;
	ChildFolderData root = { ui64Id, strPath };
	queChildFolder->push(root);

	while (queChildFolder->size())
	{
		intptr_t  FileHandle = 0;          //�ļ���� 
		struct _finddatai64_t FileInfo;		//�ļ���Ϣ
		vector<string> ChildFolder;					//��ǰĿ¼�µ���Ŀ¼
		uint64 ui64FolderSize = 0;			//��Ŀ¼�����ļ��ܴ�С
		ChildFolderData folder_data = queChildFolder->front();
		string FileName;
		if ((FileHandle = _findfirsti64(FileName.assign(folder_data.path).append("\\*").c_str(), &FileInfo)) != -1)
		{
			do
			{
				//�����Ŀ¼,����ɨ�����;�������,�����б�  
				if ((FileInfo.attrib &  _A_SUBDIR))
				{
					if (strcmp(FileInfo.name, ".") != 0 && strcmp(FileInfo.name, "..") != 0)
					{
						string strChildFolderPath = FileName.assign(folder_data.path).append("\\").append(FileInfo.name);
						ChildFolder.push_back(strChildFolderPath);
					}
				}
				else
				{
					if (bFirstGetIn)
					{
						string strCurrentFileInfo = FileName.assign(folder_data.path).append("\\").append(FileInfo.name).append("  ").append(_w2s(FormatSize(FileInfo.size).c_str())).append("\n");
						vectFileSizeInfo->push_back(strCurrentFileInfo);
					}
					ui64FolderSize += FileInfo.size;
				}
			} while (0 == _findnext64(FileHandle, &FileInfo));
			queChildFolder->pop();
			_findclose(FileHandle);
		}
		else
		{
			queChildFolder->pop();
			continue;
		}

		//����id�ж��Ǹ�Ŀ¼������56λΪ0�����Ǹ�Ŀ¼
		if (!((ui64Id) & 0x00ffffff) && bFirstGetIn)
		{
			bFirstGetIn = false;
			mapFolderSize->insert(pair<uint64, uint64>(ui64Id, 0));
			//����һ����Ŀ¼id
			for (int i = 0; i < ChildFolder.size(); ++i)
			{
				uint64 id = ui64Id + i + 1; //ǰ8λΪ����id,��56λΪ�ļ���id
				ChildFolderData data = { id, ChildFolder[i] };
				queChildFolder->push(data);
				mapFolderSize->insert(pair<uint64, uint64>(id, 0));
				mapFolderString->insert(pair<uint64, string>(id, ChildFolder[i]));
			}
		}
		else
		{
			for (int i = 0; i < ChildFolder.size(); ++i)
			{
				ChildFolderData folder = { folder_data.id,  ChildFolder[i] };
				queChildFolder->push(folder);
			}
		}

		map<uint64, uint64>::iterator iterFolderSize;
		iterFolderSize = mapFolderSize->find(folder_data.id);
		if (iterFolderSize != mapFolderSize->end())
		{
			iterFolderSize->second += ui64FolderSize;
		}

		ChildFolder.clear();
	}

	//������
	uint64 TotalSize = 0;
	string strShowFolder = "";
	string strShowFile = "";
	for (auto size : *mapFolderSize)
	{
		//�����ܴ�С
		TotalSize += size.second;

		//���������Ϣ
		map<uint64, string>::iterator iterFolderInfo;
		iterFolderInfo = mapFolderString->find(size.first);
		if (iterFolderInfo != mapFolderString->end())
		{
			wstring FolderSize = FormatSize(size.second);
			strShowFolder.append(iterFolderInfo->second).append("	").append(_w2s(FolderSize)).append("\n");
		}
	}

	for (auto file : *vectFileSizeInfo)
	{
		strShowFile.append(file);
	}

	string strShowRoot = "·��";
	strShowRoot = strShowRoot.append("\"").append(strPath.c_str()).append("\"").append("�ܴ�С:").append(_w2s(FormatSize(TotalSize))).append("\n");

	g_WriteMutex.Lock();
	cout << strShowRoot;
	cout << strShowFolder;
	cout << strShowFile;
	cout << "--------------------------------------------------" << endl;
	g_WriteMutex.Unlock();

	vectFileSizeInfo->clear();
	delete vectFileSizeInfo;

	mapFolderSize->clear();
	delete mapFolderSize;

	mapFileInfo->clear();
	delete mapFileInfo;

	mapFolderString->clear();
	delete mapFolderString;

	delete queChildFolder;

	//֪ͨ���̱߳���ɨ�����
	g_ThreadFinishEvent.Signal();

}

int _tmain(int argc, _TCHAR* argv[])
{
	// ������Ļ��������С(��λ:�ַ���)
	HANDLE hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD BuffSize;
	BuffSize.X = 300;
	BuffSize.Y = 6000;
	SetConsoleScreenBufferSize(hConsoleHandle, BuffSize);

	DWORD nMode;
	HANDLE hConsoleHandle2 = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(hConsoleHandle2, &nMode);
	SetConsoleMode(hConsoleHandle2, nMode & ~ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT);

	CThreadManage* pThreadManage = new CThreadManage(10);
	double  dStartTime = 0, dFinishTime = 0;
	map<string, uint64> mapUserInput; //�û����������·��:<·��������id>
	uint64 nWorkNum;
INPUT:
	{

		nWorkNum = 0;

		SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
		cout << "��ÿ������һ��·�������롰start����ʼ���������롰end����������" << endl;
		SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

		string strCurrentInput; //��ǰ�����ַ���
		while (getline(cin, strCurrentInput))
		{
			if (!Trim(strCurrentInput)) //ȥ�������ַ�
				continue;
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
			std::pair< std::map<string, uint64>::iterator, bool > ret;
			ret = mapUserInput.insert(pair<string, uint64>(strCurrentInput, (nWorkNum << 56)));
			if (nWorkNum >= MAX_SEARCH_PATH)
			{
				cout << "��ʼɨ��..." << endl;
				break;
			}
			else if(ret.second)
				++nWorkNum;
		}
	}

	dStartTime = clock();//ȡ��ʼʱ��

	for (auto path_map : mapUserInput)
	{
		string path = FormatDirPath(path_map.first);
		CScanFile* job = new CScanFile(path);
		job->ui64Id = path_map.second;
		job->SetJobNo(path_map.second);
		job->SetJobName(path.c_str());
		string *data = &path;
		pThreadManage->Run(job, data);
	}

	while (nWorkNum)
	{
		g_ThreadFinishEvent.Wait();
		--nWorkNum;
	}

	dFinishTime = clock();//ȡ����ʱ��

	double time = (dFinishTime - dStartTime) / CLOCKS_PER_SEC;
	cout << "����ɨ����ʱ��" << time << "seconds" << endl;
	cout << "*******************************************************" << endl;

	//�ص�����״̬
	mapUserInput.clear();
	goto INPUT;

END:
	{
		pThreadManage->TerminateAll();
		delete pThreadManage;
		pThreadManage = NULL;
	}
	return 0;
}
#endif







