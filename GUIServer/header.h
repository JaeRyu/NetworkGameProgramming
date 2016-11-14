#pragma once

#pragma once

#pragma comment (lib, "ws2_32")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//#define ISOLATION_AWARE_ENABLED 1
#define BUFSIZE 1024
#define SERVERPORT 8822

#include <WinSock2.h>

#include <windows.h>
#include <CommCtrl.h>
#include <stdio.h>
#include <sys\stat.h>

struct sThread {
	HWND hWnd;
	SOCKET sock;
	HINSTANCE hInst;
	char* filename;
};


struct Answer {
	char type;
	bool answer;
};
