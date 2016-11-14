#include "header.h"
#include "mThread.h"

// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수


HINSTANCE hInst; // 인스턴스 핸들
//HWND hEdit; // 편집 컨트롤

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
	HWND hWnd = CreateWindow("MyWndClass", "Server", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 250, 150, NULL, NULL, hInstance, NULL);
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
	static WSADATA wsa;
	static SOCKET listen_sock;
	static int retval;
	static sThread *as;
	switch (uMsg) {
	case WM_CREATE:

		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			MessageBox(NULL, "네트워크에러", NULL, MB_OK);
			exit(1);
		}

		listen_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (listen_sock == INVALID_SOCKET) err_quit("socket()");

		// bind()
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		serveraddr.sin_port = htons(SERVERPORT);
		retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) err_quit("bind()");

		retval = listen(listen_sock, SOMAXCONN);
		if (retval == SOCKET_ERROR) err_quit("listen()");

		as = new sThread;
		as->hInst = hInst;
		as->hWnd = hWnd;
		as->sock = listen_sock;

		CreateThread(NULL, 0, ProcessClient, (LPVOID)as, 0, NULL);
				
		return 0;
	case WM_PAINT:
	{
		HDC hdc;
		PAINTSTRUCT ps;
		hdc = BeginPaint(hWnd, &ps);

		TextOut(hdc, 70, 40, "서버 구동중", 11);

		EndPaint(hWnd, &ps);
	}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}