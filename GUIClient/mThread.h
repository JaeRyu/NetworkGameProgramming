#pragma once

#include "header.h"

// ���� �Լ� ���� ����
void err_quit(char *msg);

// ���� �Լ� ���� ���
void err_display(char *msg);

DWORD WINAPI SendRecvThread(LPVOID arg);

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags);