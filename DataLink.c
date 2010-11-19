#include "DataLink.h"

VOID ProcessWrite(HWND hWnd, PSTATEINFO psi) {
    PWNDDATA    pwd                     = NULL;
    OVERLAPPED  ol                      = {0};
    BYTE        pEnq[CTRL_CHAR_SIZE]    = {0};
    pwd     = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    pEnq[0] = ENQ;

    WriteFile(pwd->hPort, pEnq, CTRL_CHAR_SIZE, NULL, &ol);
    psi->iState = STATE_T1;
    srand(GetTickCount());
    psi->dwTimeout = rand() % TOR1 + 200;             // adjust this later
}


VOID ProcessRead(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {  
    
    static VOID(*pReadFunc[READ_STATES])(HWND, PSTATEINFO, BYTE*, DWORD) 
            = { ReadT1, ReadT3, ReadIDLE, ReadR2 };
    // call the read function related to the current state
    pReadFunc[psi->iState](hWnd, psi, pReadBuf, dwLength);
}


VOID ReadT1(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[0] == ACK) {
        WriteFile(pwd->hPort, TEXT("MYFILE"), CTRL_CHAR_SIZE, NULL, &ol);
        psi->iState     = STATE_T3;
		psi->itoCount   = 0;
		psi->dwTimeout  = TOR2;
//        SetEvent(OpenEvent(DELETE | SYNCHRONIZE, FALSE, TEXT("fillFTPBuffer")));
    } else {
        psi->iState     = STATE_IDLE;
    }
}


VOID ReadT3(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[0] == ACK) {
        //if (ftpQueueSize == 0)
        // WriteFile(pwd->hPort, TEXT("PUTA EOT"), CTRL_CHAR_SIZE, NULL, &ol);
    } else {
        //Get next frame
        SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("fillftpBuffer")));
        WriteFile(pwd->hPort, TEXT("A FRAME"), CTRL_CHAR_SIZE, NULL, &ol);
		psi->itoCount = 0;
    }
}


VOID ReadIDLE(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[0] == ENQ) {
        WriteFile(pwd->hPort, TEXT("AN ACK"), CTRL_CHAR_SIZE, NULL, &ol);
        psi->iState     = STATE_R2;
        psi->dwTimeout  = TOR3;
    }
}


VOID ReadR2(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[0] == EOT) {
        psi->iState = STATE_IDLE;
    } 
    //else if (portToQueueSize >= FULL_BUFFER) {
        // clear buffer
    //}
    else if (crcFast(pReadBuf, dwLength) == 0) {
 //       if (FileToPortQueueSize) {
 //           WriteFile(pwd->hPort, TEXT("AN RVI"), CTRL_CHAR_SIZE, NULL, &ol);
   //     } else {
            WriteFile(pwd->hPort, TEXT("AN ACK"), CTRL_CHAR_SIZE, NULL, &ol);
            psi->iState = STATE_T1;
     //   }
    }
}


VOID ProcessTimeout(PSTATEINFO psi) {
   
    switch (psi->iState) {
        
        case STATE_T1:
            psi->dwTimeout  = INFINITE;
            psi->iState     = STATE_IDLE;
            return;

        case STATE_T3:
            psi->dwTimeout *= (DWORD) pow((long double) 2, ++(psi->itoCount));
            psi->iState     = (psi->itoCount >= 3) ? STATE_IDLE : STATE_T2;
            return;
        
        case STATE_R2:
            psi->dwTimeout *= (DWORD) pow(2.0, ++(psi->itoCount));
            psi->iState     = (psi->itoCount >= 3) ? STATE_IDLE : STATE_R2;
            return;
        
        default:
            DISPLAY_ERROR("Invalid state for timeout");
            return;
    }
}
VOID OpenFileTransmit(HWND hWnd){
	PWNDDATA pwd = {0};
	OPENFILENAME ofn;
	char szFile[100] = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);


	GetOpenFileName(&ofn);
	pwd->lpszTransmitName = ofn.lpstrFile;
	pwd->hFileTransmit =CreateFile(pwd->lpszTransmitName, GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
							OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
	//MessageBox(hWnd, pwd->lpszTransmitName, "File", MB_OK);
	
}

VOID CloseFileTransmit(HWND hWnd){
	PWNDDATA pwd = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
	
	if(pwd->hFileTransmit){
		CloseHandle(pwd->hFileTransmit);
		pwd->lpszTransmitName = "";
		SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
		
	}
}

VOID OpenFileReceive(HWND hWnd){
	PWNDDATA pwd = {0};
	OPENFILENAME ofn;
	char szFile[100] = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nMaxFile = sizeof(szFile);


	GetSaveFileName(&ofn);
	pwd->lpszReceiveName = ofn.lpstrFile;
	pwd->hFileReceive = CreateFile(pwd->lpszReceiveName, GENERIC_READ | GENERIC_WRITE,
									FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
									OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
	
}

VOID CloseFileReceive(HWND hWnd){
	PWNDDATA pwd = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
	if(pwd->hFileReceive){
		CloseHandle(pwd->hFileReceive);
		pwd->lpszReceiveName = "";
		SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
		
	}
}

VOID WriteToFile(HWND hWnd, PFRAME frame){
	PWNDDATA pwd = {0};
	DWORD dwBytesWritten = 0;
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);

	if(!WriteFile(pwd->hFileReceive, frame->payload, frame->length, &dwBytesWritten, NULL)){
		DISPLAY_ERROR("Failed to write to file");
	}
}

VOID ReadFromFile(HWND hWnd){
	PWNDDATA pwd = {0};
	DWORD dwBytesRead = 0;
	CHAR* ReadBuffer = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);

	if(!ReadFile(pwd->hFileTransmit, ReadBuffer, 1019, &dwBytesRead, NULL)){
		DISPLAY_ERROR("Failed to read from file");
	}
	//File Empty
	if(dwBytesRead < 1019){
		pwd->bMoreData = FALSE;
		CloseFileTransmit(hWnd);
	}
	SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
	//Call createFrame(ReadBuffer, dwBytesRead);
}