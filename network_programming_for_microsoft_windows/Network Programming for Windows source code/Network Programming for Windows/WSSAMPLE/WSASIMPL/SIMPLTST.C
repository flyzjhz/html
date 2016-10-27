/*---------------------------------------------------------------------
 *
 * Program: WSASIMPL DLL Test Client
 *
 * filename: simpltst.c
 *
 * copyright by Bob Quinn, 1995
 *
 * Description:
 *  This application uses the simplified WinSock API provided by the
 *  sample WSASIMPL dynamic link library.  It connects to the echo
 *  port, sends a string of characters, then reads them back and
 *  displays them.
 *
 *  This software is not subject to any  export  provision  of
 *  the  United  States  Department  of  Commerce, and may be
 *  exported to any country or planet.
 *
 *  Permission is granted to anyone to use this  software  for any  
 *  purpose  on  any computer system, and to alter it and redistribute 
 *  it freely, subject to the following  restrictions:
 *
 *  1. The author is not responsible for the consequences of
 *     use of this software, no matter how awful, even if they
 *     arise from flaws in it.
 *
 *  2. The origin of this software must not be misrepresented,
 *     either by explicit claim or by omission.  Since few users
 *     ever read sources, credits must appear in the documentation.
 *
 *  3. Altered versions must be plainly marked as such, and
 *     must not be misrepresented as being the original software.
 *     Since few users ever read sources, credits must appear in
 *     the documentation.
 *
 *  4. This notice may not be removed or altered.
 *	 
 ---------------------------------------------------------------------*/
#include "wsasimpl.h"
#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include "..\winsockx.h"
#include "..\wsa_xtra.h" 
#include <string.h> /* for _fmemcpy() & _fmemset() */
#include "resource.h"

/*------------ global variables ------------*/
char szAppName[] = "simpltst";

#define MY_BUF_SIZE 2048
#define MY_TIMEOUT  10000

SOCKET hSock = INVALID_SOCKET;   /* Socket */
char achInBuf  [MY_BUF_SIZE];    /* Input Buffer */
char szHost [MAXHOSTNAME];       /* Echo Server to connect to */

int  wChargenLen = MY_BUF_SIZE;  /* length of string to be output */
char achChargenBuf[MY_BUF_SIZE]; /* we put CHARGEN_SEQ string resource here */
int  cbBufSize;
long cbBytesToSend = 1048576;    /* 1 Megabyte */

/* display buffer (8K) */
#define DISP_LINE_LEN  128
#define DISP_NUM_LINES 64
#define DISP_BUFSIZE   DISP_LINE_LEN * DISP_NUM_LINES

HANDLE hInst;
HWND   hWinMain;

LONG WINAPI WndProc (HWND, UINT, WPARAM,LPARAM);
BOOL WINAPI Dlg_Host  (HWND,UINT,UINT,LONG);

/*--------------------------------------------------------------------
 *  Function: WinMain()
 *
 *  Description: initialize and start message loop
 */
int PASCAL WinMain
  (HANDLE hInstance,
   HANDLE hPrevInstance,
   LPSTR  lpszCmdLine,
   int    nCmdShow)
{
    MSG msg;
    WNDCLASS wc;
    
    lpszCmdLine = lpszCmdLine; /* avoid warning */
    
    hInst = hInstance;         /* Save instance handle */
    
    if (!hPrevInstance) {
      /* register window class */
      wc.style         = CS_HREDRAW | CS_VREDRAW;
      wc.lpfnWndProc   = WndProc;
      wc.cbClsExtra    = 0;
      wc.cbWndExtra    = 0;
      wc.hInstance     = hInst;
      wc.hIcon         = LoadIcon(hInst, (LPCSTR)SIMPLTST);
      wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
      wc.hbrBackground = COLOR_WINDOW+1;
      wc.lpszMenuName  = MAKEINTRESOURCE(SIMPLTST);
      wc.lpszClassName = szAppName;
       
       if (!RegisterClass (&wc)) {
          return (0);
        }
    }
    hWinMain = CreateWindow(
        szAppName,
        "Simple Echo Client",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        500,
        300,
        NULL,
        NULL,
        hInst,
        NULL
    );                                                    
    if (!hWinMain)
        return (0);
         
    ShowWindow(hWinMain, nCmdShow);
    UpdateWindow(hWinMain);

    while (GetMessage (&msg, NULL, 0, 0)) {    /* main loop */
        TranslateMessage(&msg);
        DispatchMessage (&msg);
    }
    return msg.wParam;
    
} /* end WinMain() */

/*--------------------------------------------------------------------
 * Function: WndProc()
 *
 * Description: Main window's message processor.
 */
LONG CALLBACK WndProc
  (HWND hwnd,
   UINT msg,
   WPARAM wParam,
   LPARAM lParam)
{
    BOOL bRet;
    switch (msg) {
      case WM_COMMAND:
        switch (wParam) {
           case IDM_CONNECT:
             if (hSock == INVALID_SOCKET) {
               bRet = DialogBox (hInst, MAKEINTRESOURCE(IDD_HOST), 
		       hwnd, Dlg_Host);
               if (bRet) {
                 hSock = ConnectTCP ((LPSTR)szHost, (LPSTR)"echo");
                 if (hSock != INVALID_SOCKET) {
                 	wsprintf (achTempBuf, 
			 	  		"ConnectTCP() returned Socket %d", hSock);
                 	MessageBox (hwnd, achTempBuf, "ConnectTCP()", MB_OK);
			 	  }
               } else {
                 MessageBox (hwnd, "Need a destination host to connect to", 
                  "Can't connect!", MB_OK | MB_ICONASTERISK);
               }
             } else {
			   MessageBox (hwnd, "You already have a connection open",
			     "Can't connect!", MB_OK | MB_ICONASTERISK);
			 }
             break;
             
           case IDM_START:
             if (hSock == INVALID_SOCKET) {
               MessageBox (hwnd, "You need to connect first", 
                "Can't start sending!", MB_OK | MB_ICONASTERISK);
             } else {
               HDC hdc = GetDC(hwnd);
               int nRet = 1, i;
               long cbBytesSent = 0L;
               
               for (i=40;i;i--)
                 TextOut(hdc,(6*i),50,"  ",2);
               TextOut(hdc,10,10,"Now sending and receiving 1 megabyte",36);
               
               /*-----------------------------------------------------------
                *  Note: due to subclassing that prevents "reentrant"
                *  mouse and keyboard messages, the user cannot interact
                *  with the program while this send/receive loop executes.
                -----------------------------------------------------------*/
               while (nRet && (nRet != SOCKET_ERROR)) {
                 nRet = SendData(hSock, achChargenBuf, MY_BUF_SIZE);
                 if (nRet && (nRet != SOCKET_ERROR)) {
                   cbBytesSent += nRet;
                   nRet = RecvData(hSock, achInBuf, MY_BUF_SIZE, MY_TIMEOUT);
                   TextOut(hdc,10+(6*i),30,">",1);
                   if (i++ > 40) {
                     for (;i;i--)
                       TextOut(hdc,5+(6*i),30,"  ",2);
                   }
                 }
                 if (cbBytesSent >= cbBytesToSend)
                   break;
               }
               if (nRet && (nRet != SOCKET_ERROR))
                  TextOut(hdc,10,50,"Send and receive complete!",26);
               ReleaseDC(hwnd, hdc);
             }
             break;
           
           case IDM_CLOSE:
             if (hSock != INVALID_SOCKET) {
               CloseTCP(hSock, 0, 0);
               wsprintf (achTempBuf, 
			  		"Connection on socket %d is now closed", hSock);
               MessageBox (hwnd, achTempBuf, "CloseTCP()", MB_OK);
               hSock = INVALID_SOCKET;
             } else  {
			   MessageBox (hwnd, "No open connection to close", "Can't close!", 
			     MB_OK | MB_ICONASTERISK);
			 }
             break;
            
           case IDM_ABOUT:
             DialogBox (hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, Dlg_About);
             break;
               
           case IDM_EXIT:
             PostMessage(hwnd, WM_CLOSE, 0, 0L);
             break;
         } /* end case WM_COMMAND: */
        break;

      case WM_CREATE:
        /*------------load string into our work buffer---------- */
        cbBufSize = LoadString (hInst, CHARGEN_SEQ, achInBuf, MY_BUF_SIZE);
        if (cbBufSize) {  /* get our char string */
          int i,j;
	  
          /* number of iterations we need to copy string */
          i = (MY_BUF_SIZE) / cbBufSize;
	  
	      /* fill chargen buf w/ repeated char series */
          for (j=0; i > 0; i--, j+=cbBufSize) {  
             _fmemcpy((LPSTR)&(achChargenBuf[j]), (LPSTR)achInBuf, cbBufSize);
          }
	      /* to finish, fill remainder of buffer (if any) */
          i = (MY_BUF_SIZE) % cbBufSize;  
          if (i) {
             j = ((MY_BUF_SIZE) / cbBufSize) * cbBufSize;
             _fmemcpy ((LPSTR)&(achChargenBuf[j]), (LPSTR)achInBuf, i);
          }
        }
        /* center dialog box */
        CenterWnd (hwnd, NULL, TRUE);
        break;
 
      case WM_CLOSE:
        if (hSock != INVALID_SOCKET) {
          CloseTCP(hSock, 0, 0);                               
        }
        DestroyWindow(hwnd);
        break;

      case WM_DESTROY:
        PostQuitMessage(0);
        break;
             
    default:
        return (DefWindowProc(hwnd, msg, wParam, lParam));   
  } /* end switch (msg) */
  return 0;
} /* end WndProc() */

/*---------------------------------------------------------------------
 * Function: Dlg_Host()
 *
 * Description: Prompt user for destination host or address.
 */                                        
BOOL WINAPI Dlg_Host (
  HWND hDlg,
  UINT msg,
  UINT wParam,
  LPARAM lParam)
{
  BOOL bRet = FALSE;
  
  lParam = lParam;  /* avoid warning */
  
  switch (msg) {
    case WM_INITDIALOG:
      /* set display values */
      SetDlgItemText (hDlg, IDC_HOST, szHost);
      SetFocus (GetDlgItem (hDlg, IDC_HOST));

      /* center dialog box */
      CenterWnd (hDlg, NULL, TRUE);
      break;
    case WM_COMMAND:
      switch (wParam) {
        case IDOK:
          GetDlgItemText (hDlg, IDC_HOST, szHost, MAXHOSTNAME);
          EndDialog (hDlg, TRUE);
          bRet = TRUE;
          break;
        case IDCANCEL:
          EndDialog (hDlg, FALSE);
          bRet = FALSE;
          break;
        default:
          break;
      }
  }        
  return (bRet);
} /* end Dlg_Host() */

/*---------------------------------------------------------------------
 * Function: Dlg_About()
 *
 * Description: Dialog procedure for About box.  We cannot use our
 *  WinSockx library Dlg_About() procedure because we don't have
 *  a WSAData structure to display (we don't call WSAStartup(),
 *  but the WSASIMPL DLL does).
 */
BOOL FAR PASCAL Dlg_About (
	HWND hDlg,
	UINT msg,
	UINT wParam,
	LONG lParam)
{
	char achDataBuf[WSADESCRIPTION_LEN+1];
	
	lParam = lParam;	/* avoid warning */

    switch (msg) {
        case WM_INITDIALOG:
  			wsprintf (achDataBuf, "(Compiled: %s, %s)\n", 
  				(LPSTR)__DATE__, (LPSTR)__TIME__);
            SetDlgItemText (hDlg, IDC_COMPILEDATE, (LPCSTR)achDataBuf);
            /* center dialog box */
            CenterWnd (hDlg, NULL, TRUE);
	    	return FALSE;
	    
		case WM_COMMAND:
	    	switch (wParam) {
	        	case IDOK:
	            	EndDialog (hDlg, 0);
	            	return TRUE;
	    }
	break;
    }
    return FALSE;
} /* end Dlg_About() */

