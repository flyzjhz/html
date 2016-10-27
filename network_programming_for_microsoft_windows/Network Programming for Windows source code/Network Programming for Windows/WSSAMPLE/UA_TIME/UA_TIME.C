/*---------------------------------------------------------------------
 *
 *  Program: UA_TIME.EXE  Asynch UDP Time Client and Server
 *
 *  filename: ua_time.c
 *  
 *  copyright by Bob Quinn, 1995
 *   
 *  Description:
 *    Client and Server application that uses and provides "daytime"
 *    service as described by RFC 867.  Using UDP this simple service
 *    responds to an empty datagram received on port 13 by returning
 *    the current date and time as an ASCII character string.
 *
 *  This software is not subject to any  export  provision  of
 *  the  United  States  Department  of  Commerce,  and may be
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
#include <windows.h>
#include <winsock.h> /* 32 and 16-bit Windows Sockets */
#include <string.h>  /* for _fmemcpy() & _fmemset() */
#include <time.h>

#include "..\winsockx.h"
#include "..\wsa_xtra.h" 
#include "resource.h"

/* WinSock Version (1.1) */
#define WS_VERSION_REQD	0x0101

/* our asynch notification message */                             
#define WSA_ASYNC WM_USER+1

/* timeout id and period (in milliseconds) */
#define TIMEOUT_ID     1
#define TIMEOUT_PERIOD 30000 

#define BUF_SIZE 1024
#define ERR_SIZE 512
                             
/*------------ global variables ------------*/
WSADATA stWSAData;  /* WinSock DLL Info */

char szAppName[] = "UA_Time";

BOOL bRecieving = FALSE;  /* state flag */

SOCKET hSock = INVALID_SOCKET;  /* socket handle */
char szHost[MAXHOSTNAME] = {0}; /* remote host string (name or address) */

SOCKADDR_IN stLclName;  /* local socket name (address & port) */
SOCKADDR_IN stRmtName;  /* remote socket name (address & port) */

char achInBuf  [BUF_SIZE];  /* Input Buffer */
char achOutBuf [BUF_SIZE];  /* Output Buffer */

BOOL bBroadcast = FALSE;    /* Broadcast enabled flag */

HWND hwndMain;        /* Main window handle */
HINSTANCE hInst;      /* Instance handle */

/*------------- function prototypes --------------*/
LONG CALLBACK WndProc (HWND,UINT,WPARAM,LONG);
BOOL CALLBACK DestDlgProc  (HWND,UINT,UINT,LONG);

/*--------------------------------------------------------------------
 *  Function: WinMain()
 *
 *  Description: Initialize and start message loop
 *
 */
int PASCAL WinMain
  (HANDLE hInstance,
   HANDLE hPrevInstance,
   LPSTR  lpszCmdLine,
   int    nCmdShow)
{
    MSG msg;
    int nRet;
    WNDCLASS  wndclass;

    lpszCmdLine = lpszCmdLine; /* avoid warning */
                      
    hInst = hInstance;  /* save instance handle */

    if (!hPrevInstance) {
      /* register window class */
      wndclass.style         = CS_HREDRAW | CS_VREDRAW;
      wndclass.lpfnWndProc   = WndProc;
      wndclass.cbClsExtra    = 0;
      wndclass.cbWndExtra    = 0;
      wndclass.hInstance     = hInstance;
      wndclass.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(UA_TIME));
      wndclass.hCursor       = LoadCursor(NULL,IDC_ARROW);
      wndclass.hbrBackground = COLOR_WINDOW+1;
      wndclass.lpszMenuName  = MAKEINTRESOURCE(UA_TIME);
      wndclass.lpszClassName = szAppName;
       
      if (!RegisterClass (&wndclass)) {
        return (0);
      }
    }
    
    hwndMain = CreateWindow(
        szAppName,
        "Daytime Client & Server",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        400,
        200,
        NULL,
        NULL,
        hInstance,
        NULL
    );                                                    

    if (!hwndMain)   /* quit now if class registration failed */
        return 0;
    /*-------------initialize WinSock DLL------------*/
    nRet = WSAStartup(WS_VERSION_REQD, &stWSAData);
    /* WSAStartup() returns error value if failed (0 on success) */
    if (nRet != 0) {    
      WSAperror(nRet, "WSAStartup()", hInst);
      /* No sense continuing if we can't use WinSock */
    } else {
          
      ShowWindow(hwndMain, nCmdShow);   /* display our window */
      UpdateWindow(hwndMain);
                             
      while (GetMessage (&msg, NULL, 0, 0)) {  /* main message loop */
        TranslateMessage(&msg);
        DispatchMessage (&msg);
      }
    
      /*---------------release WinSock DLL--------------*/
      nRet = WSACleanup();
      if (nRet == SOCKET_ERROR)
        WSAperror(WSAGetLastError(), "WSACleanup()", hInst);
    }    

    /* Return resource explicitly */   
    UnregisterClass(szAppName, hInstance); 
        
    return msg.wParam;
} /* end WinMain() */

/*--------------------------------------------------------------------
 * Function: WndProc()
 *
 * Description: Main window procedure, handles asynch messages.
 */
LONG CALLBACK WndProc 
  (HWND hwnd,
   UINT msg,
   WPARAM wParam,
   LONG lParam)
{
    FARPROC lpfnProc;
    int nAddrSize = sizeof(SOCKADDR);
    WORD WSAEvent, WSAErr, wCmd;
    HMENU hMenu;
    int nRet;
   
    switch (msg) {
      case WSA_ASYNC:
        /* We received a WSAAsyncSelect() FD_ notification message 
         *  Parse the message to extract FD_ event value and error
         *  value (if there is one).
         */
        WSAEvent = WSAGETSELECTEVENT (lParam);
        WSAErr   = WSAGETSELECTERROR (lParam);
        if (WSAErr) {
          /* Error in asynch notification message: display to user */
          WSAperror(WSAErr,"FD_READ", hInst);
          /* fall-through to call reenabling function for this event */
        }
        switch (WSAEvent) {
		  case FD_READ:
            /* Receive the available data */
            nRet = recvfrom (hSock, (char FAR *)achInBuf, BUF_SIZE, 0,
              (struct sockaddr *) &stRmtName, &nAddrSize);

            /* Display error if receive failed (but don't repeat error
             *  if input message contained an error) */
            if ((nRet == SOCKET_ERROR) && (!WSAErr)) {
              WSAperror(WSAErr,"recvfrom()", hInst);
              break;
            }

            if (bRecieving) {
              /*--------------------------------------------- 
               * CLIENT: 
               * If we sent a request, display the response */
              achInBuf[nRet-2] = 0; /* remove CR/LF */
              /* Display the data received, and who it came from */
              wsprintf ((LPSTR)achOutBuf, "%s : %s",
                inet_ntoa(stRmtName.sin_addr), (LPSTR)achInBuf);
              MessageBox (hwnd, (LPSTR)achOutBuf, 
                "Daytime", MB_OK | MB_ICONASTERISK);
                
              /* Remove the timeout alert */
              KillTimer (hwnd, TIMEOUT_ID);
                
              /* Reset the socket state */
              bRecieving = FALSE;
            } else {
              /*---------------------------------------------- 
               * SERVER: 
               *  Send time to host we received request from */
              time_t stTime;
              time (&stTime);
              wsprintf (achOutBuf, "%s", ctime(&stTime));
              nRet = sendto (hSock, achOutBuf, strlen(achOutBuf), 0,
                  (LPSOCKADDR)&stRmtName, sizeof(SOCKADDR));
              if (nRet == SOCKET_ERROR)
                WSAperror(WSAGetLastError(), "sendto()", hInst);
            }
            break;
          default:
             break;
        } /* end switch(WSAEvent) */
        break;
      case WM_COMMAND:
#ifdef WIN32
        wCmd = LOWORD(wParam);
#else
        wCmd = (WORD)wParam;
#endif
        switch (wCmd) {
          case IDM_OPEN:
             /* If we already have a socket open, then close it first */
             if (hSock != INVALID_SOCKET) {
               nRet = closesocket(hSock);
               hSock = INVALID_SOCKET;
             }
             if (nRet == SOCKET_ERROR)               
               WSAperror(WSAGetLastError(), "socket()", hInst);
                
             /* Get a UDP socket */
             hSock = socket (AF_INET, SOCK_DGRAM, 0);
             if (hSock == INVALID_SOCKET)  {
               WSAperror(WSAGetLastError(), "socket()", hInst);
             } else {
               int ok = TRUE;
               
               /* Request async notification for data arrival. */
               nRet = WSAAsyncSelect(hSock, hwnd, WSA_ASYNC, FD_READ);
               if (nRet == SOCKET_ERROR) {
                 WSAperror(WSAGetLastError(), "WSAAsyncSelect()", hInst);
                 ok = FALSE;
               }
               
               if (ok) {  
                 /* Name the socket, so we can receive requests as a server */
                 stLclName.sin_family = PF_INET;
                 stLclName.sin_port   = htons(IPPORT_DAYTIME);
                 stLclName.sin_addr.s_addr = INADDR_ANY;
                 nRet = bind(hSock,(LPSOCKADDR)&stLclName, sizeof(struct sockaddr));  
                 if (nRet == SOCKET_ERROR) {
                   WSAperror(WSAGetLastError(), "bind()", hInst);
                   ok = FALSE;
                 }
               }
               
               if (ok) {
                 wsprintf ((LPSTR)achOutBuf, 
                 "Socket %d, named and registered for FD_READ", hSock);
                 MessageBox (hwnd, (LPSTR)achOutBuf, 
                   "Ready to Send or Receive", MB_OK | MB_ICONASTERISK);
               }
             }
           break;
             
           case IDM_SENDTO:
             /* Create Dialog box to prompt for destination host */
             lpfnProc = MakeProcInstance((FARPROC)DestDlgProc,hInst);
             nRet = DialogBox (hInst, "DESTINATIONDLG", hwndMain, lpfnProc);
             FreeProcInstance((FARPROC) lpfnProc);

             /* Check the destination address and resolve if if necessary */
             stRmtName.sin_addr.s_addr = GetAddr((LPSTR)szHost);
             if (stRmtName.sin_addr.s_addr == INADDR_ANY) { 
               if (nRet != -1)
                 /* Tell them they need to enter a host (unless they cancelled) */
                 MessageBox (hwnd, "Need a destination host to send to", 
                   "Can't connect!", MB_OK | MB_ICONASTERISK);
             } else {
               /* Set Timer so we can give up after waiting a while */
               if (!SetTimer (hwnd, TIMEOUT_ID, TIMEOUT_PERIOD, NULL))
                 MessageBox (hwnd, "SetTimer failed", "Error", MB_OK | MB_ICONASTERISK);
               
               /* Set socket state to indicate we're waiting for a response as a client */
               bRecieving = TRUE;
             
               /* send a dummy datagram to daytime port to request daytime response */
               stRmtName.sin_family = PF_INET;
               stRmtName.sin_port   = htons (IPPORT_DAYTIME);
               nRet = sendto(hSock, (char FAR *)achOutBuf, 1, 0, 
                             (LPSOCKADDR)&stRmtName, sizeof(SOCKADDR));
               if (nRet == SOCKET_ERROR)
                 WSAperror(WSAGetLastError(),"sendto()", hInst);
             }
             break;
            
           case IDM_BROADCAST:
             /* Call setsockopt() SO_BROADCAST to enable or disable */
             hMenu = GetMenu(hwnd);
             bBroadcast = !CheckMenuItem (hMenu, IDM_BROADCAST, 
                 (bBroadcast ? MF_UNCHECKED : MF_CHECKED));
             nRet = setsockopt(hSock, SOL_SOCKET, SO_BROADCAST, 
                 (LPSTR)&bBroadcast, sizeof(BOOL));
             if (nRet == SOCKET_ERROR)
               WSAperror (WSAGetLastError(), "setsockopt()", hInst); 
             break;
        
           case IDM_ABOUT:
             DialogBox (hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, Dlg_About);
             break;
               
           case IDM_EXIT:
             PostMessage(hwnd, WM_CLOSE, 0, 0L);
             break;
                       
           default:
             return (DefWindowProc(hwnd, msg, wParam, lParam));
         } /* end case WM_COMMAND: */
         break;
           
      case WM_TIMER:
        /* Timeout occurred */
        bRecieving = FALSE;          /* reset state */
        KillTimer (hwnd, TIMEOUT_ID);  /* release timer */
        MessageBox (hwnd, "No response from daytime server", 
           "Timeout", MB_OK | MB_ICONASTERISK);
        break;

      case WM_QUERYENDSESSION:
      case WM_CLOSE:
        /* Close the socket before we leave. */
        if (!SendMessage(hwnd, WM_COMMAND, IDM_CLOSE, 1L))
          DestroyWindow(hwnd);
        /* Release Timer (if it's active) */
        if (bRecieving)
          KillTimer(hwndMain, TIMEOUT_ID);
        break;

      case WM_CREATE:
        /* center dialog box */
        CenterWnd (hwnd, NULL, TRUE);
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
 * Function: DestDlgProc()
 *
 * Description: Prompt user for destination (hostname or address).
 */                                        
BOOL CALLBACK DestDlgProc (
  HWND hDlg,
  UINT msg,
  UINT wParam,
  LONG lParam)
{
  static int wRet, nOptName, nOptVal, nOptLen, nOptIDC, nLevel, WSAerr;
  static struct linger stLinger;
  
  lParam = lParam;  /* avoid warning */
  
   switch (msg) {
     case WM_INITDIALOG:
       /* set display values */
       SetDlgItemText (hDlg, IDC_DESTADDR, szHost);
       SetFocus (GetDlgItem (hDlg, IDC_DESTADDR));

       /* center dialog box */
       CenterWnd (hDlg, hwndMain, TRUE);

       return FALSE;
        
     case WM_COMMAND:
       switch (wParam) {

       case IDOK:
          GetDlgItemText (hDlg, IDC_DESTADDR, szHost, MAXHOSTNAME);
          EndDialog (hDlg, TRUE);
          
       case IDCANCEL:
          EndDialog (hDlg, -1);
          break;
     }
     return(TRUE);
   }        
   return(FALSE);
    
} /* end DestDlgProc() */

