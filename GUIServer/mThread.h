#pragma once

#include"header.h"

// ���� �Լ� ���� ����
void err_quit(char *msg);

// ���� �Լ� ���� ���
void err_display(char *msg);

DWORD WINAPI SendThread(LPVOID arg);
DWORD WINAPI SendRecvThread(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);
DWORD WINAPI DialogThread(LPVOID arg);
BOOL CALLBACK DlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
//LRESULT CALLBACK WndProgress(HWND, UINT, WPARAM, LPARAM);


// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags);