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
map<string, uint64> mapUserInput; //用户输入的任务路径:<路径，任务id>

queue<ChildFolderData> queSearchPath;  //要搜索的子目录路径

map<uint64, uint64> mapFolderSize;	//记录各个一级子目录大小:<文件夹id，文件夹大小>
map<uint64, string> mapFolderString; //记录各个一级子目录路径信息:<文件夹id，文件夹路径>

//vector<uint64> vectForderSize; //各子文件夹大小
//vector<string> vectFileInfo;  //各文件大小

map<uint64, vector<string>> mapFileInfo;

bool GetRootContent[MAX_SEARCH_PATH] = {false};

CThreadMutex g_WriteMutex;
CThreadCondition g_ThreadFinishEvent;

void CScanFile::Run(void* jobData)
{
	string path = strPath;

	uint64 ui64FolderSize = 0;			//该目录下子文件总大小
	intptr_t  FileHandle = 0;          //文件句柄                          
	struct _finddatai64_t FileInfo;		//文件信息
	string FileName = "";
	vector<string> ChildFolder;					//当前目录下的子目录
	vector<string> vectFileSizeInfo;			//当前目录要打印的下文件信息
	if ((FileHandle = _findfirsti64(FileName.assign(path).append("\\*").c_str(), &FileInfo)) != -1)
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
		//cout << "路径 \"" << path << "\" 未找到！" << endl;
		return;
	}

	g_WriteMutex.Lock();

	//根据id判断是根目录：若低56位为0，则是根目录
	if ( !((ui64Id) & 0x00ffffff) && !GetRootContent[(ui64Id>>56)])
	{
		GetRootContent[(ui64Id >> 56)] = true;
		mapFolderSize.insert(pair<uint64, uint64>(ui64Id, 0));
		//生成一级子目录id
		for (int i = 0; i < ChildFolder.size(); ++i)
		{
			uint64 id = ui64Id + i + 1; //前8位为任务id,后56位为文件夹id
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

	//通知主线程本次扫描结束
	g_ThreadFinishEvent.Signal();
}

 int _tmain(int argc, _TCHAR* argv[])
 {
	 // 设置屏幕缓冲区大小(单位:字符数)
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
		 cout << "请每行输入一个路径，输入“start”开始搜索，输入“end”结束搜索" << endl;
		 SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

		 string strCurrentInput; //当前输入字符串
		 while (getline(cin, strCurrentInput))
		 {
			 if (!Trim(strCurrentInput)) //去除控制字符
				 continue;
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
			 mapUserInput.insert(pair<string, uint64>(strCurrentInput, (nWorkNum<<56)));
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
		 DWORD result = g_ThreadFinishEvent.Wait(200);
		 if (result == WAIT_TIMEOUT && !queSearchPath.size() && !pThreadManage->GetBusyThreadNum())//超时，且待扫描队列为空，忙碌线程也为空，则认为扫描结束
		 	break;
	 }
	 dFinishTime = clock();//取开始时间
#endif // SINGLE_THREAD

	 //输出结果
	 {
		 //计算和打印可以拆开成多个线程
		 for (auto path : mapUserInput)
		 {
			 uint64 TotalSize = 0;
			 string strShowFolder = "";
			 string strShowFile = "";
			 for (auto size : mapFolderSize)
			 {
				 if ((size.first>>56) == (path.second>>56)) //搞8位id一致，说明子目录归属于该根目录
				 {
					 //加入总大小
					 TotalSize += size.second;

					 //加入输出信息
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

			 string strShowRoot = "路径";
			 strShowRoot = strShowRoot.append("\"").append(path.first.c_str()).append("\"").append("总大小:").append(_w2s(FormatSize(TotalSize))).append("\n");

			 cout << strShowRoot;
			 cout << strShowFolder;
			 cout << strShowFile;
			 cout << "--------------------------------------------------" << endl;
		 }
		 double time = (dFinishTime - dStartTime) / CLOCKS_PER_SEC;
		 cout << "本次扫描用时："<<time<<"seconds" << endl;
		 cout << "*******************************************************" << endl;
	 }

	 //回到输入状态
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

	vector<string>* vectFileSizeInfo = new vector<string>;			//当前目录要打印的下文件信息

	map<uint64, uint64>* mapFolderSize = new map<uint64, uint64>;	//记录各个一级子目录大小:<文件夹id，文件夹大小>
	map<uint64, vector<string>>* mapFileInfo = new map<uint64, vector<string>>;
	map<uint64, string>* mapFolderString = new map<uint64, string>; //记录各个一级子目录路径信息:<文件夹id，文件夹路径>

	queue<ChildFolderData>* queChildFolder = new queue<ChildFolderData>;
	ChildFolderData root = { ui64Id, strPath };
	queChildFolder->push(root);

	while (queChildFolder->size())
	{
		intptr_t  FileHandle = 0;          //文件句柄 
		struct _finddatai64_t FileInfo;		//文件信息
		vector<string> ChildFolder;					//当前目录下的子目录
		uint64 ui64FolderSize = 0;			//该目录下子文件总大小
		ChildFolderData folder_data = queChildFolder->front();
		string FileName;
		if ((FileHandle = _findfirsti64(FileName.assign(folder_data.path).append("\\*").c_str(), &FileInfo)) != -1)
		{
			do
			{
				//如果是目录,加入扫描队列;如果不是,加入列表  
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

		//根据id判断是根目录：若低56位为0，则是根目录
		if (!((ui64Id) & 0x00ffffff) && bFirstGetIn)
		{
			bFirstGetIn = false;
			mapFolderSize->insert(pair<uint64, uint64>(ui64Id, 0));
			//生成一级子目录id
			for (int i = 0; i < ChildFolder.size(); ++i)
			{
				uint64 id = ui64Id + i + 1; //前8位为任务id,后56位为文件夹id
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

	//输出结果
	uint64 TotalSize = 0;
	string strShowFolder = "";
	string strShowFile = "";
	for (auto size : *mapFolderSize)
	{
		//加入总大小
		TotalSize += size.second;

		//加入输出信息
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

	string strShowRoot = "路径";
	strShowRoot = strShowRoot.append("\"").append(strPath.c_str()).append("\"").append("总大小:").append(_w2s(FormatSize(TotalSize))).append("\n");

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

	//通知主线程本次扫描结束
	g_ThreadFinishEvent.Signal();

}

int _tmain(int argc, _TCHAR* argv[])
{
	// 设置屏幕缓冲区大小(单位:字符数)
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
	map<string, uint64> mapUserInput; //用户输入的任务路径:<路径，任务id>
	uint64 nWorkNum;
INPUT:
	{

		nWorkNum = 0;

		SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
		cout << "请每行输入一个路径，输入“start”开始搜索，输入“end”结束搜索" << endl;
		SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

		string strCurrentInput; //当前输入字符串
		while (getline(cin, strCurrentInput))
		{
			if (!Trim(strCurrentInput)) //去除控制字符
				continue;
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
			std::pair< std::map<string, uint64>::iterator, bool > ret;
			ret = mapUserInput.insert(pair<string, uint64>(strCurrentInput, (nWorkNum << 56)));
			if (nWorkNum >= MAX_SEARCH_PATH)
			{
				cout << "开始扫描..." << endl;
				break;
			}
			else if(ret.second)
				++nWorkNum;
		}
	}

	dStartTime = clock();//取开始时间

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

	dFinishTime = clock();//取结束时间

	double time = (dFinishTime - dStartTime) / CLOCKS_PER_SEC;
	cout << "本次扫描用时：" << time << "seconds" << endl;
	cout << "*******************************************************" << endl;

	//回到输入状态
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







