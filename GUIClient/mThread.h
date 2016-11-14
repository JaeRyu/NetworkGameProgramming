#pragma once

#include "header.h"

// 소켓 함수 오류 종료
void err_quit(char *msg);

// 소켓 함수 오류 출력
void err_display(char *msg);

DWORD WINAPI SendRecvThread(LPVOID arg);

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags);