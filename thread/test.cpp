#include "stdafx.h"
#include "tool.h"

// unsigned int __stdcall thrd_start_routine(PVOID v)
// {
// 	Sleep(6000);
// 	printf("created thread\n");
// 	return 0;
// }
// 
// #ifdef _TEST
// 
// int main()
// {
// 	HWND thrdid = (HWND)_beginthreadex(NULL, 0, thrd_start_routine, NULL, 0, NULL);
// 	Sleep(3000);
// 	printf("main thread\n");
// 
// 	cout << "����Ҫ�������·������ÿ������һ��·�������롰start����ʼ���������롰end����������" << endl;
// 	vector<string> search_path;  //Ҫ������·��
// 	string current_input_string; //��ǰ�����ַ���
// 
// 	while (getline(cin, current_input_string))
// 	{
// 		if (current_input_string == "end")
// 		{
// 			return  0;
// 		}
// 		else if (current_input_string == "start")
// 		{
// 			if (!search_path.size())
// 			{
// 				SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
// 				cout << "����·��Ϊ�գ����������룡" << endl;
// 				SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
// 				continue;
// 			}
// 			else
// 			{
// 				start_search(search_path);
// 			}
// 			break;
// 		}
// 
// 		search_path.push_back(current_input_string);
// 	}
// }
// 
// #endif // _TEST
