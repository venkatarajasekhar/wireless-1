/*------------------------------------------------------------------------------
-- SOURCE FILE:     Presentation.c - Contains all the OSI "presentation layer"
--                                   functions for the program.
--
-- PROGRAM:     Dean and the Rockets' Wireless Protocol Testing and Evaluation 
--              Facilitator
--
-- FUNCTIONS:
--              VOID    UpdateDisplayBuf(HWND hWnd, CHAR cCharacter);
--              VOID    HorizontalTab(HWND hWnd);
--              VOID    FormFeed(HWND hWnd);
--              VOID    MoveCursor(HWND hWnd, INT cxCoord, INT cyCoord, 
--                                 BOOL bScroll);
--              VOID    ClearLine(HWND hWnd, UINT cxCoord, UINT cyCoord, 
--                                INT iDirection);
--              VOID    ClearScreen(HWND hWnd, UINT cxCoord, UINT cyCoord, 
--                                  INT iDirection);
--              VOID    ScrollDown(HWND hWnd);
--              VOID    ScrollUp(HWND hWnd);
--              VOID    SetScrollRegion(HWND hWnd, INT cyTop, INT cyBottom); 
--				VOID 	DisplayFrameInfo(HWND hWnd, FRAME frame);
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   November 4, 2010 - ProcessPacket, EchoTag
--              November 7, 2010 - Removed a number of unecessary functions.
--				December 2, 2010 - Removed 2 more unecessary functions.
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- NOTES:
-- Contains presentation level functions for the program.
-- These are the functions that process the characters both typed and received
-- into meaningful data.
------------------------------------------------------------------------------*/
#include "Presentation.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    UpdateDisplayBuf
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID UpdateDisplayBuf(HWND hWnd, CHAR cCharacter)
--                          hWnd        - the handle to the window
--                          cCharacter  - the character to add to the buffer
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Adds cCharacter to the display buffer.
------------------------------------------------------------------------------*/
VOID UpdateDisplayBuf(HWND hWnd, CHAR cCharacter) {
    
    PWNDDATA    pwd     = NULL;
    CHAR        a[2]    = {0};    
    HDC         hdc     = {0};
    CHARINFO    ci      = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    a[0] = cCharacter;
     
    CHARACTER(X, Y).character   = cCharacter;
    CHARACTER(X, Y).fgColor     = CUR_FG_COLOR;
    CHARACTER(X, Y).bgColor     = CUR_BG_COLOR;
    CHARACTER(X, Y).style	    = CUR_STYLE;
    
    if (X >= CHARS_PER_LINE - 1) { 
        if (pwd->wordWrap == FALSE) {
            return;
        }
        X = 0;
        if (Y < LINES_PER_SCRN - 1) { 
            Y++;
        } else {
            ScrollDown(hWnd);
        }
    } else {
        X++;
    }
}
/*------------------------------------------------------------------------------
-- FUNCTION:    DisplayFrameInfo
--
-- DATE:        Dec 2, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Daniel Wright
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   DisplayFrameInfo(HWND hWnd, FRAME frame)
--                      hWnd			- a handle to the window
--						frame			- frame to display information about
--
-- RETURNS:     VOID.
--
-- NOTES:       Uses GetOpenFileName to get the name of the file to be
--				transmitted and opens it.
------------------------------------------------------------------------------*/
VOID DisplayFrameInfo(HWND hWnd, FRAME frame){
    PWNDDATA	pwd		= NULL;
    
    int a ;
    CHAR* b = (CHAR*)malloc(sizeof(CHAR)*4);
    CHAR* c = (CHAR*)malloc(sizeof(CHAR));
    int i = 0;
    int j;
    SetScrollRegion(hWnd,2,LINES_PER_SCRN);
    ScrollUp(hWnd);

    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    a= pwd->NumOfFrames;
    sprintf(b, "%04x", frame.length);
    sprintf(c, "%02x", frame.crc);
    
    MoveCursor( hWnd, 3,  2, FALSE);
    for(i = 0; i < 3; i++){
        j=(a)%(10);
        a /=10;
        UpdateDisplayBuf(hWnd, j+48);
        X -=2;
    }
    MoveCursor( hWnd, 12,  2, FALSE);
    for(i = 0; i < 4; i++)
        UpdateDisplayBuf(hWnd, b[i]);
    MoveCursor(hWnd, 29,  2, FALSE);
    for(i = 0; i < 3; i++){
        UpdateDisplayBuf(hWnd, c[i]);
    }
    InvalidateRect(hWnd,NULL,FALSE);
    
}


/*------------------------------------------------------------------------------
-- FUNCTION:    HorizontalTab
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID HorizontalTab(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Moves the cursor to the next tab on the line.
------------------------------------------------------------------------------*/
VOID HorizontalTab(HWND hWnd) {
    PWNDDATA pwd = NULL;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);   
    if (X < CHARS_PER_LINE - TAB_LENGTH) {
        X += TAB_LENGTH - X % TAB_LENGTH;
    } else {
        X = CHARS_PER_LINE - 1;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    FormFeed
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID FormFeed(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Clears the screen and moves the cursor to 1, 1 (0, 0 according
--              to the display buffer).
------------------------------------------------------------------------------*/
VOID FormFeed(HWND hWnd) { 
    PWNDDATA    pwd = NULL;
    UINT        i   = 0;
    UINT        j   = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    for (i = 0; i < LINES_PER_SCRN; i++) {
        for (j = 0; j < CHARS_PER_LINE; j++) {
            CHARACTER(j, i).character   = ' ';
            CHARACTER(j, i).bgColor     = CUR_BG_COLOR;
            CHARACTER(j, i).style       = 0;
         }
    }
    X = 0;
    Y = 0;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    MoveCursor
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID MoveCursor(HWND hWnd, UINT cxCoord, UINT cyCoord, 
--                              BOOL bScroll)
--                          hWnd        - the handle to the window
--                          cxCoord     - the x coordinate to move the cursor to
--                                        (1,1) origin
--                          cyCoord     - the y coordinate to move the cursor to
--                                        (1,1) origin
--                          bScroll     - whether or not to scroll when the
--                                        y coordinate is beyond the bounds of
--                                        screen
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Moves the cursor to the specified position. The position passed 
--              in the arguments is based screen coordinates(1,1) origin).
--              If bScroll is true, then the screen will scroll when the top
--              of bottom lines of the window are reached.
------------------------------------------------------------------------------*/
VOID MoveCursor(HWND hWnd, INT cxCoord, INT cyCoord, BOOL bScroll) {
    PWNDDATA pwd = NULL;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    if (cxCoord < 1) {
        X = 0;
    } else if (cxCoord > CHARS_PER_LINE) {
        X = CHARS_PER_LINE - 1;
    } else {
        X = --cxCoord;
    }

    if (cyCoord < WINDOW_TOP + 1) {
        Y = WINDOW_TOP;
        if (bScroll) {
            ScrollUp(hWnd);
        }
    } else if (cyCoord > WINDOW_BOTTOM + 1) {
        Y = WINDOW_BOTTOM;
        if (bScroll) {
            ScrollDown(hWnd);
        }
    } else {
        Y = --cyCoord;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ClearLine
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID ClearLine(HWND hWnd, UINT cxCoord, UINT cyCoord, 
--                             INT iDirection)
--                          hWnd        - the handle to the window
--                          cxCoord     - the row of the first 
--                                        character to clear - (0,0) origin
--                          cyCoord     - the line of the first
--                                        character to clear - (0,0) origin
--                          iDirection  - the direction (left or right) to clear
--                                        the line
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Clears a line in the direction specified by iDirection (left or
--              right). The character under the cursor will be cleared as well.
--              Please note that cxCoord and cyCoord use a (0,0) origin.
------------------------------------------------------------------------------*/
VOID ClearLine(HWND hWnd, UINT cxCoord, UINT cyCoord, INT iDirection) {
    PWNDDATA    pwd = NULL;
    UINT        i   = 0;
    UINT        j   = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    i = cyCoord;
    j = cxCoord;
    while (j < CHARS_PER_LINE  &&  j >= 0) {
        CHARACTER(j, i).character   = ' ';
        CHARACTER(j, i).bgColor     = CUR_BG_COLOR;
        CHARACTER(j, i).style       = 0;
        j += iDirection;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ClearScreen
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID ClearScreen(HWND hWnd, UINT cxCoord, UINT cyCoord, 
--                               INT iDirection)
--                          hWnd        - the handle to the window
--                          cxCoord     - the row of the first 
--                                        character to clear - (0,0) origin
--                          cyCoord     - the line of the first
--                                        character to clear - (0,0) origin
--                          iDirection  - the direction (up or down) to clear
--                                        the screen
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Clears the screen in the direction specified by iDirection (all
--              preceding characters, or all following characters). The
--              character under the cursor will be cleared as well. Please note
--              that cxCoord and cyCoord use a (0,0) origin.
------------------------------------------------------------------------------*/
VOID ClearScreen(HWND hWnd, UINT cxCoord, UINT cyCoord, INT iDirection) {
    PWNDDATA    pwd = NULL;
    UINT        i   = 0;
    UINT        j   = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    ClearLine(hWnd, cxCoord, cyCoord, iDirection);

    i = cyCoord + iDirection;
    while (i < LINES_PER_SCRN  &&  i >= 0) {
        for (j = 0; j < CHARS_PER_LINE; j++) {
            CHARACTER(j, i).character   = ' ';
            CHARACTER(j, i).bgColor     = CUR_BG_COLOR;
            CHARACTER(j, i).style       = 0;
         }
         i += iDirection;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ScrollDown
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID ScrollDown(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              "Scrolls down" one line. It moves every line on the screen up
--              one position, deleting the top line, and creating a new, blank
--              bottom line.
------------------------------------------------------------------------------*/
VOID ScrollDown(HWND hWnd) {
    PWNDDATA    pwd         = NULL;
    PLINE       pNewLine    = NULL;
    INT         i           = 0;
    INT         j           = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0); 
    pNewLine = (PLINE) malloc(sizeof(LINE));
    free(ROW(WINDOW_TOP));

    for (i = WINDOW_TOP; i < WINDOW_BOTTOM; i++) {
        ROW(i) = ROW(i + 1);
    }
    ROW(i) = pNewLine;
    for (j = 0; j < CHARS_PER_LINE; j++) {
        CHARACTER(j, i).character   = ' ';
        CHARACTER(j, i).bgColor     = CUR_BG_COLOR;
        CHARACTER(j, i).style       = 0;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ScrollUp
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID ScrollUp(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              "Scrolls up" one line. It moves every line on the screen down
--              one position, deleting the bottom line, and creating a new, 
--              blank top line.
------------------------------------------------------------------------------*/
VOID ScrollUp(HWND hWnd) {
    PWNDDATA    pwd         = NULL;
    PLINE       pNewLine    = NULL;
    INT         i           = 0;
    INT         j           = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0); 
    pNewLine = (PLINE) malloc(sizeof(LINE));
    free(ROW(WINDOW_BOTTOM));
    
    for (i = WINDOW_BOTTOM; i > WINDOW_TOP; i--) {
        ROW(i) = ROW(i - 1);
    }
    ROW(i) = pNewLine;
    for (j = 0; j < CHARS_PER_LINE; j++) {
        CHARACTER(j, i).character   = ' ';
        CHARACTER(j, i).bgColor     = CUR_BG_COLOR;
        CHARACTER(j, i).style       = 0;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    SetScrollRegion
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID SetScrollRegion(HWND hWnd, INT cyTop, INT cyBottom)
--                          hWnd     - the handle to the window
--                          cyTop    - the top line of the new scroll range
--                          cyBottom - the bottom line of the new scroll range
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Sets the top and bottom lines for the scrollable region. This
--              allows the appearance of scrolling for one section of lines,
--              whereas the other lines will remain motionless.
------------------------------------------------------------------------------*/
VOID SetScrollRegion(HWND hWnd, INT cyTop, INT cyBottom) {
    PWNDDATA pwd = NULL;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0); 
    MoveCursor(hWnd, 1, cyTop, FALSE);
    WINDOW_TOP      = --cyTop;
    WINDOW_BOTTOM   = --cyBottom;   
}
