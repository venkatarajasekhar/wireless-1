/*------------------------------------------------------------------------------
-- SOURCE FILE:     FileIO.c      Contains functions relating to File IO for
--									the 3980 wireless protocol.
--                      
-- PROGRAM:     Dean and the Rockets' Wireless Protocol Testing and Evaluation 
--              Facilitator
--
-- FUNCTIONS:
--              FRAME	CreateFrame(HWND hWnd, PBYTE psBuf, DWORD dwLength);
--				VOID 	OpenFileTransmit(HWND hWnd);
--				VOID 	CloseFileTransmit(HWND hWnd);
--				BOOL 	OpenFileReceive(HWND hWnd);
--				VOID 	CloseFileReceive(HWND hWnd);
--				VOID 	WriteToFile(HWND hWnd);
--				VOID 	ReadFromFile(HWND hWnd);
--				FRAME 	CreateNullFrame(HWND hWnd);
--
--
-- DATE:        Dec 2, 2010
--
-- REVISIONS:   
--
-- DESIGNER:    Dean Morin/Daniel Wright/Ian Lee
--
-- PROGRAMMER:  Daniel Wright/Ian Lee
--
-- NOTES:       Functions relating to File IO
------------------------------------------------------------------------------*/
#include "FileIO.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    CreateFrame
--
-- DATE:        Nov 24, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Ian Lee
--
-- PROGRAMMER:  Ian Lee
--
-- INTERFACE:   CreateFrame(HWND hWnd, PBYTE psBuf, DWORD dwLength)
--                      hWnd			- a handle to the window
--						PBYTE psBuf		- data from file
--						DWORD dwLength	- length of data from file
--
-- RETURNS:     FRAME - the created frame
--
-- NOTES:       Creates a frame to be put into the FTP buffer
--
------------------------------------------------------------------------------*/
FRAME CreateFrame(HWND hWnd, PBYTE psBuf, DWORD dwLength){
    DWORD		i;
    FRAME myFrame;
    PWNDDATA    pwd                 = NULL;

    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    myFrame.soh = 0x1;
    myFrame.sequence = pwd->TxSequenceNumber;
    pwd->TxSequenceNumber= (pwd->TxSequenceNumber==0)?1:0;
    myFrame.length = (SHORT)dwLength;

    for (i = 0; i<dwLength;i++) {
        myFrame.payload[i] = *(psBuf++);
    }
    while(i<MAX_PAYLOAD_SIZE){
        myFrame.payload[i++] =0;
    }
    myFrame.crc =0;
    myFrame.crc = crcFast((PBYTE)&myFrame,FRAME_SIZE - sizeof(crc));
    if(crcFast((PBYTE)&myFrame,FRAME_SIZE)!=0){
        DISPLAY_ERROR("Failure Creating Frame");
    }
    
    return myFrame;
}
/*------------------------------------------------------------------------------
-- FUNCTION:    OpenFileTransmit
--
-- DATE:        Dec 2, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Daniel Wright
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   OpenFileTransmit(HWND hWnd)
--                      hWnd			- a handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:       Uses GetOpenFileName to get the name of the file to be
--				transmitted and opens it.
------------------------------------------------------------------------------*/
VOID OpenFileTransmit(HWND hWnd){
    PWNDDATA pwd = {0};
    OPENFILENAME ofn;
    TCHAR szFile[100] = {0};
    pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = TEXT('\0');
    ofn.lpstrFilter = TEXT("All\0*.*\0");
    ofn.nMaxFile = sizeof(szFile);


    if(GetOpenFileName(&ofn) == 0){
        return;
    }
    
    pwd->hFileTransmit =CreateFile(ofn.lpstrFile, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    pwd->NumOfReads = 0;
    PostMessage(hWnd, WM_FILLFTPBUF, 0, 0);
    pwd->TxSequenceNumber = 0;
}
/*------------------------------------------------------------------------------
-- FUNCTION:    CloseFileTransmit
--
-- DATE:        Dec 2, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Daniel Wright
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   CloseFileTransmit(HWND hWnd)
--                      hWnd			- a handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:       Closes the file handle to the file to be transmitted
--				and sets it to null. Sends message to fill File to Port
--				Buffer.
------------------------------------------------------------------------------*/
VOID CloseFileTransmit(HWND hWnd){
    PWNDDATA pwd = {0};
    pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
    
    if(pwd->hFileTransmit != NULL){
        if(!CloseHandle(pwd->hFileTransmit)){
            DISPLAY_ERROR("Failed to close Transmit File");
        }
        pwd->hFileTransmit = NULL;
        pwd->NumOfReads = 0;
    }
}
/*------------------------------------------------------------------------------
-- FUNCTION:    OpenFileReceive
--
-- DATE:        Dec 2, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Daniel Wright
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   OpenFileReceive(HWND hWnd)
--                      hWnd			- a handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:       Uses GetSaveFileName to get the name of the file for data
--				to be written to and opens it.
------------------------------------------------------------------------------*/
BOOL OpenFileReceive(HWND hWnd){
    PWNDDATA pwd = {0};
    OPENFILENAME ofn;
    TCHAR szFile[100] = {0};                  	
    pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = TEXT('\0');
    ofn.lpstrFilter = TEXT("All\0*.*\0");
    ofn.nMaxFile = sizeof(szFile);


    if(GetSaveFileName(&ofn) == 0){
        return FALSE;
    } 
    
    pwd->hFileReceive = CreateFile(ofn.lpstrFile, GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
    return TRUE;
}
/*------------------------------------------------------------------------------
-- FUNCTION:    CloseFileReceive
--
-- DATE:        Dec 2, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Daniel Wright
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   CloseFileReceive(HWND hWnd)
--                      hWnd			- a handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:       Closes the handle to the receive file and sets it to null.
------------------------------------------------------------------------------*/
VOID CloseFileReceive(HWND hWnd){
    PWNDDATA pwd = {0};
    pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
    if(pwd->hFileReceive){
        if(!CloseHandle(pwd->hFileReceive)){
            DISPLAY_ERROR("Failed to close Receive File");
        }
        pwd->hFileReceive = NULL;
            
    }
}
/*------------------------------------------------------------------------------
-- FUNCTION:    WriteToFile
--
-- DATE:        Dec 2, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Daniel Wright
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   WriteToFile(HWND hWnd)
--                      hWnd			- a handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:       Removes frames from the Port to File Queue and writes them
--				to the receive file until the queue is empty.
------------------------------------------------------------------------------*/
VOID WriteToFile(HWND hWnd){
    PWNDDATA pwd = {0};
    DWORD dwBytesWritten = 0;
    PFRAME		tempFrame = {0};
    HANDLE		hMutex = {0};
    pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
    
    while(pwd->PTFQueueSize != 0){
        hMutex = CreateMutex(NULL, FALSE, TEXT("PTFMutex"));
        WaitForSingleObject(hMutex,INFINITE);
        tempFrame = RemoveFromFrameQueue(&pwd->PTFBuffHead, 1);
        ReleaseMutex(hMutex);
        if(tempFrame->length != 0){
            DisplayFrameInfo(hWnd, *tempFrame);
            pwd->NumOfFrames++;
            SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
        }
        
        if(!WriteFile(pwd->hFileReceive, tempFrame->payload, tempFrame->length, &dwBytesWritten, NULL)){
            DISPLAY_ERROR("Failed to write to file");
        } else {
            pwd->PTFQueueSize--;
        }
        
        if(tempFrame->length != MAX_PAYLOAD_SIZE ){		
            CloseFileReceive(hWnd);
            MessageBox(hWnd, TEXT("Finished receiving file"), 
					   TEXT("Transfer Complete"), MB_OK);
        }
    }
}
/*------------------------------------------------------------------------------
-- FUNCTION:    ReadFromFile
--
-- DATE:        Dec 2, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Daniel Wright
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   ReadFromFile(HWND hWnd)
--                      hWnd			- a handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:       While the File to Port queue is not full, reads in increments
--				of 1019 bytes from the file to be transmitted, calls CreateFrame
--				to frame them and adds them to the File to Port queue.
------------------------------------------------------------------------------*/
VOID ReadFromFile(HWND hWnd){
    PWNDDATA pwd = {0};
    DWORD dwBytesRead = 0;
    DWORD dwBytesWritten = 0;
    DWORD	dwSizeOfFile = 0;
    BOOL	eof	= FALSE;
    FRAME frame;
    HANDLE hMutex = {0};
    int i;
    PBYTE ReadBuffer = (PBYTE) malloc(sizeof(BYTE) *1019);

    pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
    
    if (!(dwSizeOfFile = GetFileSize(pwd->hFileTransmit, NULL))) {
        return;
    }

    while(pwd->FTPQueueSize < FULL_BUFFER && pwd->hFileTransmit != NULL){

        if((i =dwSizeOfFile - ((pwd->NumOfReads) * 1019)) > 0){
            if(!ReadFile(pwd->hFileTransmit, ReadBuffer, 1019, &dwBytesRead, NULL)){
                DISPLAY_ERROR("Failed to read from file");
            }
            ++pwd->NumOfReads;
            frame = CreateFrame(hWnd, ReadBuffer, dwBytesRead);

        } else if((dwSizeOfFile - ((pwd->NumOfReads) * 1019)) == 0){
            CloseFileTransmit(hWnd);
            ++pwd->NumOfReads;
            

            frame = CreateNullFrame(hWnd);
            MessageBox(hWnd, TEXT("Transmit File Buffering Complete"), 
				       TEXT("File Read Complete"), MB_OK);

        } else {
            
            CloseFileTransmit(hWnd);
            MessageBox(hWnd, TEXT("Transmit File Buffering Complete"), 
				       TEXT("File Read Complete"), MB_OK);
            return;
        } 
                
        hMutex = CreateMutex(NULL, FALSE, TEXT("FTPMutex"));
        WaitForSingleObject(hMutex,INFINITE);
        AddToFrameQueue(&pwd->FTPBuffHead, &pwd->FTPBuffTail, frame);
        pwd->FTPQueueSize+=1;

        ReleaseMutex(hMutex);
    }
}
/*------------------------------------------------------------------------------
-- FUNCTION:    CreateNullFrame
--
-- DATE:        Dec 2, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Daniel Wright
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   CreateNullFrame(HWND hWnd)
--                      hWnd			- a handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:       Creates a frame with the payload consisting entirely of nulls,
--				to be send at the end of a file if no nulls were sent in 
--				previous frames.
------------------------------------------------------------------------------*/
FRAME CreateNullFrame(HWND hWnd){
    FRAME nullFrame = CreateFrame(hWnd, 0, 0);
    return nullFrame;
}
