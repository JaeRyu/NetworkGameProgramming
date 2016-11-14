#include "header.h"
#include "mThread.h"
#include "resource.h"

// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProgress(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

WNDPROC Progress;
int Pos;


HINSTANCE hInst; // 인스턴스 핸들
HWND hEdit; // 편집 컨트롤
DWORD SERVERIP;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;

	// 윈도우 클래스 등록
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "MyWndClass";
	if (!RegisterClass(&wndclass)) return 1;

	// 윈도우 생성
	HWND hWnd = CreateWindow("MyWndClass", "Client", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 200, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) return 1;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// 메시지 루프
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static RECT rcClient;
	static int cyVScroll;

	
	static WSADATA wsa;
	static SOCKET sock;
	char str[300];
	char lpstrFile[MAX_PATH] = "";
	static sThread *s;
	static int retval;

	switch (uMsg) {
	case WM_CREATE:

		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, DlgProc);

		//소켓 초기화
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			MessageBox(NULL, "네트워크에러", NULL, MB_OK);
			exit(1);
		}

		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET) err_quit("socket()");


		//서버 연결
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = htonl(SERVERIP);
		serveraddr.sin_port = htons(SERVERPORT);
		retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) err_quit("connect()");


		GetClientRect(hWnd, &rcClient);
		cyVScroll = GetSystemMetrics(SM_CYVSCROLL);
		hEdit = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL,
			WS_CHILD | WS_VISIBLE, 
			10, 120, 360, 20, 
			hWnd, (HMENU)0, hInst, NULL);
		
		
		Progress = (WNDPROC)SetWindowLong(hEdit, GWL_WNDPROC, (LONG)WndProgress);
		Pos = 0;
		SendMessage(hEdit, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendMessage(hEdit, PBM_SETPOS, Pos, 0);
		s = new sThread;
		s->hWnd = hEdit;
		s->sock = sock;


		CreateThread(NULL, 0, SendRecvThread, (LPVOID)s, 0, NULL);
		
		return 0;
	case WM_PAINT:
	{
		HDC hdc;
		PAINTSTRUCT ps;
		hdc = BeginPaint(hWnd, &ps);

		TextOut(hdc, 10, 30, "파일받기", 8);

		EndPaint(hWnd,&ps);
	}

		break;
	case WM_DESTROY:

		closesocket(sock);
		WSACleanup();
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);



}


LRESULT CALLBACK WndProgress(HWND hWndProgress, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_CREATE:
		return 0;
	}
	return CallWindowProc(Progress, hWndProgress, iMessage, wParam, lParam);
}


BOOL CALLBACK DlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			SendMessage(GetDlgItem(hDlg, IDC_IPADDRESS1), IPM_GETADDRESS, 0, (LPARAM)&SERVERIP);
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

