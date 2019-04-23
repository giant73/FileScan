#pragma once
#include <queue>

BOOL SetConsoleColor(WORD wAttributes)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE)
		return FALSE;

	return SetConsoleTextAttribute(hConsole, wAttributes);
}

std::string _w2s(const std::wstring str)
{
	unsigned len = str.size() * 4;
	setlocale(LC_CTYPE, "");
	char *p = new char[len];
	wcstombs(p, str.c_str(), len);
	std::string str1(p);
	delete[] p;
	return str1;
}

std::wstring _s2w(const std::string str)
{
	unsigned len = str.size() * 2;// Ԥ���ֽ���
	setlocale(LC_CTYPE, "");     //������ô˺���
	wchar_t *p = new wchar_t[len];// ����һ���ڴ���ת������ַ���
	mbstowcs(p, str.c_str(), len);// ת��
	std::wstring str1(p);
	delete[] p;// �ͷ�������ڴ�
	return str1;
}
string FormatDirPath(const string &path)
{
	if (path.empty())
	{
		return "";
	}

	string strPath(path);
	for (size_t i = 0; i < strPath.size(); ++i)
	{
		if (strPath[i] == '/')
			strPath[i] = '\\';
	}
	string::size_type findpos = strPath.find("\\\\");
	while (string::npos != findpos)
	{
		strPath = strPath.substr(0, findpos) + strPath.substr(findpos + 1);
		findpos = strPath.find("\\\\");
	}
	if (strPath[strPath.size()-1]  == '\\')
		strPath = strPath.substr(0, strPath.length() - 1);

	return strPath;
}

wstring FormatSize(uint64 size)
{
	static TCHAR str[260] = {};
	if (size < 0)
	{
		return L"0B";
	}
	else
	{
		static TCHAR* for_mat[] = { _T("%.0fB"),_T("%.2fKB"), _T("%.2fMB"), _T("%.2fGB"), _T("%.2fTB"), NULL };
		INT64 uint = 1;
		int i = 0;
		while ((size >= uint * 1000) && for_mat[i + 1] != NULL)
		{
			uint <<= 10;
			++i;
		}
		_stprintf_s(str, 259, for_mat[i], (double)size / uint);
	}
	//return str;
}

bool Trim(string &trim)
{
	for (int i = trim.length()-1; i>=0;--i)
	{
		if (trim.at(i) > 32)
			return true;
	}
	return false;
}

uint64 __stdcall ScanPath(string path, vector<string>& files) //files�洢��·���µ������ļ�
{
	uint64 ui64FolderSize = 0;			//��Ŀ¼�����ļ��ܴ�С
	long file_handle = 0;          //�ļ����                          
	struct _finddata_t file_info; //�ļ���Ϣ
	string file_name = "";
	vector<string> child_folder;	//��ǰĿ¼�µ���Ŀ¼
	if ((file_handle = _findfirst(file_name.assign(path).append("\\*").c_str(), &file_info)) != -1)
	{
		do
		{
			uint64 file_size = 0;
			//�����Ŀ¼,����֮;�������,�����б�  
			if ((file_info.attrib &  _A_SUBDIR))
			{
				if (strcmp(file_info.name, ".") != 0 && strcmp(file_info.name, "..") != 0)
				{
					string child_folder_path = file_name.assign(path).append("\\").append(file_info.name);
					file_size = ScanPath(child_folder_path, files); //�ݹ�������Ŀ¼
					child_folder.push_back(child_folder_path);
				}
			}
			else
			{
				string current_file_info = file_name.assign(path).append("\\").append(file_info.name).append("  ").append(_w2s(FormatSize(file_info.size).c_str()));
				files.push_back(current_file_info);
				file_size = file_info.size;
			}
			ui64FolderSize += file_size;
		} while (0 == _findnext(file_handle, &file_info));

		_findclose(file_handle);
	}
	else
	{
		SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
		cout << "·�� \"" << path << "\" δ�ҵ���" << endl;
		SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}
	return ui64FolderSize;
}

int StartScan(queue<string>& queSearchPath)
{
	if (queSearchPath.size())
	{
		uint64 ui64FolderSize;
		vector<string> vectFile;
		for (; queSearchPath.size();)
		{
			double  dStartTime, dFinishTime;
			dStartTime = clock();//ȡ��ʼʱ��
			string strSearchPath = FormatDirPath(queSearchPath.front());
			queSearchPath.pop();
			ui64FolderSize = ScanPath(strSearchPath, vectFile);
			wstring size = FormatSize(ui64FolderSize);

			dFinishTime = clock();//ȡ����ʱ��
			double dTime = (dFinishTime - dStartTime) / CLOCKS_PER_SEC;
			//printf("Ŀ¼ %s ��С��%s��ɨ����ʱ��%f seconds\n", n, size, (finish_time - start_time) / CLOCKS_PER_SEC);//����Ϊ��λ��ʾ
			for (auto n : vectFile)
			{
				cout << n<<endl;
			}

			cout << "Ŀ¼\"" << strSearchPath << "\"��С:" << _w2s(size).c_str() << "�� �ļ�����:" << vectFile.size() << "�� ɨ����ʱ:" << dTime << " seconds" << endl;//����Ϊ��λ��ʾ
		}
	}
	return 0;
}
