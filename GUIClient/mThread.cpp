#include "mThread.h"


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

DWORD WINAPI SendRecvThread(LPVOID arg)
{
	sThread tmp = *(sThread *)arg;
	HWND hEdit = tmp.hWnd;
	SOCKET sock = tmp.sock;
	int retval;
	char str[300];
	if (MessageBox(NULL, "파일을 전송받으시겠습니까?", "클라이언트", MB_OKCANCEL) == IDOK)
	{

		//파일명을 전송해달라고 서버에 보내기
		Answer as;
		as.answer = true;
		as.type = 0;
		retval = send(sock, (char *)&as, sizeof(as), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send(Answer0)");
		}

		int filenamesize;
		
		//파일명 길이 받기
		retval = recvn(sock, (char*)&filenamesize, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv(filenamesize)");
		}
		
		char *filename = new char[filenamesize+1];
		//파일명 받기
		retval = recvn(sock, filename, filenamesize, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv(filename)");
		}
		filename[retval] = '\0';

		//저장할곳 열기
		OPENFILENAME OFN;
		memset(&OFN, 0, sizeof(OPENFILENAME));
		OFN.lStructSize = sizeof(OPENFILENAME);
		OFN.hwndOwner = NULL;
		OFN.lpstrFilter = "Every File(*.*)\0*.*\0Text File\0*.txt;*.doc\0";
		OFN.lpstrFile = filename;
		OFN.nMaxFile = 256;
		OFN.lpstrInitialDir = "c:\\";
		if (GetSaveFileName(&OFN) != 0) {
			wsprintf(str, "%s", OFN.lpstrFile);
		}
		else
			exit(1);

		if (OFN.lpstrFile != "")
		{
			//파일 받기가 준비되었다고 서버에 보내기
			as.answer = true;
			as.type = 1;
			retval = send(sock, (char*)&as, sizeof(as), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
			}

			int len;
			// 파일 총 크기 받기
			retval = recvn(sock, (char *)&len, sizeof(int), 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv(filesize)");	
			}

			FILE *fout;
			fout = fopen(OFN.lpstrFile, "wb");
			int count = 0;
			while (1)
			{
				char ch;
				char buf[BUFSIZE];
				// 데이터 받기(고정 길이)
				retval = recvn(sock, buf, sizeof(char), 0);
				if (retval == SOCKET_ERROR) {
					err_display("recv(data size)");
					break;
				}
				else if (retval == 0)
					break;
				if (count < len)
				{
					fputc(ch, fout);
					int percent;
					percent = ((float)count / (float)len) * 100.0f;
					SendMessage(hEdit, PBM_SETPOS, percent, 0);
					count++;
				}
				if(count >= len)
				{
					int percent;
					percent = ((float)count / (float)len) * 100.0f;
					SendMessage(hEdit, PBM_SETPOS, percent, 0);
					break;
				}
			}
			fclose(fout);
		}
	}

	if (MessageBox(NULL, "파일을 받았습니다.", "클라이언트", MB_OK) == IDOK)
		exit(1);


	return 0;
}
