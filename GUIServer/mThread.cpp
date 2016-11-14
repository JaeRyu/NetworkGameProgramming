#include "mThread.h"
#include "resource.h"

HWND Dlgwnd;
char *ip;
HANDLE dlgEvent;

void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	sThread as = *(sThread *)arg;
	SOCKET listen_sock = as.sock;
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;

	OPENFILENAME OFN;
	char str[300];
	char lpstrFile[MAX_PATH] = "";

	memset(&OFN, 0, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner = NULL;
	OFN.lpstrFilter = "Every File(*.*)\0*.*\0Text File\0*.txt;*.doc\0";
	OFN.lpstrFile = lpstrFile;
	OFN.nMaxFile = 256;
	OFN.lpstrInitialDir = "";
	if (GetOpenFileName(&OFN) != 0) {
		wsprintf(str, "%s", OFN.lpstrFile);
		
	}
	
	while (1)
	{
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}
		ip = inet_ntoa(clientaddr.sin_addr);

		sThread *st = new sThread;
		st->hInst = as.hInst;
		st->hWnd = as.hWnd;
		st->filename = OFN.lpstrFile;
		st->sock = client_sock;

		CreateThread(NULL, 0, SendRecvThread, (LPVOID)st, 0, NULL);

	}
	return 0;
}


DWORD WINAPI SendThread(LPVOID arg)
{
	return 0;
}
DWORD WINAPI SendRecvThread(LPVOID arg)
{
	sThread as = *(sThread *)arg;

	SOCKET sock = as.sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	int retval;


	getpeername(sock, (SOCKADDR *)&clientaddr, &addrlen);

	//ip = inet_ntoa(clientaddr.sin_addr);
	
	//HWND hDlg = CreateDialog(as.hInst, MAKEINTRESOURCE(IDD_DIALOG1), as.hWnd, DlgProc);
	//ShowWindow(hDlg, SW_SHOW);

	dlgEvent =  CreateEvent(NULL, FALSE, FALSE, NULL);

	HANDLE dlgHandle = CreateThread(NULL, 0, DialogThread, arg, 0, NULL);
	WaitForSingleObject(dlgEvent, INFINITE);
	
	HWND hDlg = Dlgwnd;
	Answer ans;
	SetDlgItemText(hDlg, IDC_EDIT1, "클라이언트 대기중");
	while (1)
	{

		retval = recv(sock, (char*)&ans, sizeof(ans), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		if (ans.answer && ans.type == 0)
		{
			SetDlgItemText(hDlg, IDC_EDIT1, "클라이언트 응답 대기중");

			int filenamesize = strlen(as.filename);
			char strr[25];
			wsprintf(strr, "%d", filenamesize);
			
			retval = send(sock, (char *)&filenamesize, sizeof(int), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}

			retval = send(sock, as.filename, filenamesize, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}
		else if (ans.answer && ans.type == 1)
		{
			FILE *fp = fopen(as.filename, "rb");
			struct _stat64 statbuf;
			_stat64(as.filename, &statbuf);
			SetDlgItemText(hDlg, IDC_EDIT1, "파일전송중");


			long fsize = statbuf.st_size;

			int len = fsize;

			//파일 크기 보내기
			retval = send(sock, (char *)&len, sizeof(int), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send(filesize)");
			}

			int i = 0;
			int percent = 0;
			
			while (i < fsize)
			{
				//char buf[BUFSIZE];
				//fgets(buf, BUFSIZE, fp);
				char ch = fgetc(fp);
				retval = send(sock, &ch, sizeof(char), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send(file)");
					break;
				}
				HWND hProgress= GetDlgItem(hDlg, IDC_PROGRESS1);
				percent = ((float)i / (float)fsize) * 100.0f;
				SendMessage(hProgress, PBM_SETPOS, percent, 0);
				
				++i;
			}
			HWND hProgress = GetDlgItem(hDlg, IDC_PROGRESS1);
			SendMessage(hProgress, PBM_SETPOS, 100, 0);
			EndDialog(hDlg, 0);
			fclose(fp);
			break;
		}
		
	}
	return 0;
}

DWORD WINAPI DialogThread(LPVOID arg)
{
	sThread as = *(sThread *)arg;

	DialogBox(as.hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	return 0;
}


BOOL CALLBACK DlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{

	switch (iMessage)
	{
	case WM_INITDIALOG:
		Dlgwnd = hDlg;
		//SetDlgItemText(hDlg, IDC_EDIT1, ip);
		SetEvent(dlgEvent);
		break;
	case WM_COMMAND:
		switch (wParam)
		{
		}
		break;
	case WM_PAINT:
		break;
	case WM_DESTROY:
		EndDialog(hDlg, 0);
		break;
	}
	return FALSE;
}

