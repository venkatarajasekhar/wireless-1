#ifndef PHYSICAL_H
#define PHYSICAL_H

#include "List.h"
#include "osi.h"
#include "WndExtra.h"

#define READ_BUFSIZE    4096

DWORD WINAPI    PortIOThreadProc(HWND hWnd);
DWORD WINAPI	FileIOThreadProc(HWND hWnd);
VOID    ProcessCommError(HANDLE hPort);
VOID    ReadFromPort(HWND hWnd, PSTATEINFO psi, OVERLAPPED ol, DWORD cbInQue);
VOID    InitStateInfo (PSTATEINFO psi);

#endif
