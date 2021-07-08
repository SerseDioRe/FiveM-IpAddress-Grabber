#include <Windows.h>
#include <fstream>																										
#include <iomanip>																										
#include "detours.h"																									
#pragma comment(lib, "detours/detours.lib")																				
#pragma comment(lib,"ws2_32.lib")	

std::ofstream ipLog;
std::string ipAndPort;

int searchString1(const std::string& str, char searched, int sum)
{
	int index{};

	for (int i{ 0 }; i < str.size(); ++i)
	{
		if (str[i] == searched)
		{
			index = i + sum;
		}
	}

	return index;
}

int(WINAPI* oSend)(SOCKET, const char*, int, int) = send;

int WINAPI HookSend(SOCKET s, const char* buf, int len, int flags)
{
	std::string bufferStr(buf);

	if (bufferStr.find("GET /info.json HTTP/1.1") != std::string::npos)
	{
		int index{ searchString1(bufferStr, 'H', 4) };
		bufferStr.erase(bufferStr.begin(), bufferStr.begin() + index + 2);

		int index2{ searchString1(bufferStr, 'U', 0) };
		bufferStr.erase(bufferStr.begin() + index2, bufferStr.end());

		ipAndPort = bufferStr;

		return oSend(s, buf, len, flags);
	}
	else
	{
		return oSend(s, buf, len, flags);
	}
}

DWORD WINAPI HookThread(HMODULE hModule)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(LPVOID&)oSend, (PBYTE)HookSend);
	DetourTransactionCommit();

	while(true)
	{
		if(GetAsyncKeyState(VK_DELETE) & 1)
		{
			ipLog.open("C:\\ProgramData\\ipgrabbed.txt", std::ios::app);
			ipLog << ipAndPort << '\n';
			ipLog << std::endl;
			ipLog.close();
		}
	}

	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HookThread, hModule, 0, nullptr));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

