#include "stdafx.h"
#include "Job.h"
#include "ThreadManage.h"
#include "tool.h"
#include "ScanFile.h"
#include <map>
#include <queue>

#define MAX_SEARCH_PATH 16

map<string, int> mapUserInput; //用户输入的任务路径

struct ChildFolderData
{
	uint64 id;
	string path;
};

queue<ChildFolderData> queSearchPath;  //要搜索的子目录路径

map<uint64, uint64> mapFolderSize; //记录各个一级子目录大小

//vector<uint64> vectForderSize; //各子文件夹大小
vector<string> vectFileInfo;  //各文件大小
map<uint64, vector<string>> mapFileInfo;

bool GetRootContent[MAX_SEARCH_PATH] = {};

CThreadMutex g_WriteMutex;
CThreadCondition g_ThreadFinishEvent;

void CScanFile::Run(void* jobData)
{
	string path = strPath;// *(string*)jobData;

	uint64 ui64FolderSize = 0;			//该目录下子文件总大小
	intptr_t  FileHandle = 0;          //文件句柄                          
	struct _finddata_t FileInfo; //文件信息
	string FileName = "";
	vector<string> ChildFolder;					//当前目录下的子目录
	vector<string> vectFileSizeInfo;			//当前目录要打印的下文件信息
	if ((FileHandle = _findfirst(FileName.assign(path).append("\\*").c_str(), &FileInfo)) != -1)
	{
		do
		{
			uint64 ui64FileSize = 0;
			//如果是目录,加入扫描队列;如果不是,加入列表  
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
		cout << "路径 \"" << path << "\" 未找到！" << endl;
		return;
	}

	g_WriteMutex.Lock();

	if (!GetRootContent[ui64Id])
	{
		GetRootContent[ui64Id] = true;

		//生成一级子目录id
		for (int i = 0; i < ChildFolder.size(); ++i)
		{
			uint64 id = ui64Id << 24 + i + 1; //前8位为任务id,后24位为文件夹id
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

	//通知主线程本次扫描结束
	g_ThreadFinishEvent.Signal();
}

 int _tmain(int argc, _TCHAR* argv[])
 {
	 // 设置屏幕缓冲区大小(单位:字符数)
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
		 cout << "请每行输入一个路径，输入“start”开始搜索，输入“end”结束搜索" << endl;
		 string strCurrentInput; //当前输入字符串
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
					 cout << "输入路径为空，请重新输入！" << endl;
					 continue;
				 }
				 else
				 {
					 cout << "开始扫描..." << endl;				
					 break;
				 }
			 }
			 //queSearchPath.push(strCurrentInput);	
			 mapUserInput.insert(pair<string, int>(strCurrentInput, nWorkNum));
			 if (nWorkNum >= MAX_SEARCH_PATH)
			 {
				 cout << "开始扫描..." << endl;
				 break;
			 }
			 else
				 ++nWorkNum;
		 }
	 }
#ifdef SINGLE_THREAD
	 StartScan(queSearchPath);
#else

	 dStartTime = clock();//取开始时间

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
		 if (result == WAIT_TIMEOUT && !queSearchPath.size() && !pThreadManage->GetBusyThreadNum())//超时，且待扫描队列为空，忙碌线程也为空，则认为扫描结束
		 	break;
	 }
	 dFinishTime = clock();//取开始时间
#endif // SINGLE_THREAD

	 //输出结果
	 {
		 uint64 d = 0;
		 for (auto n : mapUserInput)
		 {
			 cout << "路径" << "\"" << n.first << "\":" << "总大小" << d << endl;
		 }
		 wcout << FormatSize(d) << endl;
		 double dd = (dFinishTime - dStartTime) / CLOCKS_PER_SEC;
		 cout << dd << endl;
	 }

	 //回到输入状态
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







