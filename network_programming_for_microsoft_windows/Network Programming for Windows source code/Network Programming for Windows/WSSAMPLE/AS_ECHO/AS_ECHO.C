/*---------------------------------------------------------------------
 *
 * Program: AS_ECHO.EXE  Asynch Echo Server (TCP)
 *
 * filename: as_echo.c
 *
 * copyright by Bob Quinn, 1995
 *   
 * Description:
 *  Server application that implements echo protocol service as
 *  described by RFC 862.  This application demonstrates simultaneous
 *  multiple user support using asynchronous operation mode.
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
#define STRICT
#include "..\wsa_xtra.h"
#include <windows.h>
#include <windowsx.h>

#include <winsock.h>
#include "resource.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <dos.h>
#include <direct.h>
#include "..\winsockx.h"

/*-------------- global data -------------*/
char szAppName[] = "as_echo";

SOCKET hLstnSock=INVALID_SOCKET; /* Listening socket */
SOCKADDR_IN stLclName;           /* Local address and port number */
char szHostName[MAXHOSTNAME]={0};/* Local host name */
int  iActiveConns;               /* Number of active connections */
long lByteCount;                 /* Total bytes read */
int  iTotalConns;                /* Connections closed so far */

typedef struct stConnData {
  SOCKET hSock;                  /* Connection socket */
  SOCKADDR_IN stRmtName;         /* Remote host address & port */
  LONG lStartTime;               /* Time of connect */
  BOOL bReadPending;             /* Deferred read flag */
  int  iBytesRcvd;               /* Data currently buffered */             
  int  iBytesSent;               /* Data sent from buffer */
  long lByteCount;               /* Total bytes received */
  char achIOBuf  [INPUT_SIZE];   /* Network I/O data buffer */
  struct stConnData FAR*lpstNext;/* Pointer to next record */
} CONNDATA, *PCONNDATA, FAR *LPCONNDATA;
LPCONNDATA lpstSockHead = 0;     /* Head of the list */
 
char szLogFile[] = "as_echo.log";/* Connection log file */
HFILE hLogFile=HFILE_ERROR;
BOOL  bReAsync=TRUE;

/*------------ function prototypes -----------*/
int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int);
BOOL CALLBACK Dlg_Main (HWND, UINT, UINT, LPARAM);
BOOL InitLstnSock(int, PSOCKADDR_IN, HWND, u_int);
SOCKET AcceptConn(SOCKET, PSOCKADDR_IN);
int SendData(SOCKET, LPSTR, int);
int RecvData(SOCKET, LPSTR, int);
int CloseConn(SOCKET, LPSTR, int, HWND);
void DoStats (long, long, LPCONNDATA);
LPCONNDATA NewConn (SOCKET, PSOCKADDR_IN);
LPCONNDATA FindConn (SOCKET);
void RemoveConn (LPCONNDATA);

/*--------------------------------------------------------------------
 *  Function: WinMain()
 *
 *  Description: 
 *     Initialize WinSock and display main dialog box
 */
int WINAPI WinMain
  (HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPSTR  lpszCmdLine,
   int    nCmdShow)
{
    MSG msg;
    int nRet;

    lpszCmdLine   = lpszCmdLine;   /* avoid warning */
    hPrevInstance = hPrevInstance;
    nCmdShow      = nCmdShow;
                      
    hInst = hInstance;	/* save instance handle */
                                                                
    /*-------------initialize WinSock DLL------------*/
    nRet = WSAStartup(WSA_VERSION, &stWSAData);
    /* WSAStartup() returns error value if failed (0 on success) */
    if (nRet != 0) {    
      WSAperror(nRet, "WSAStartup()", hInst);
      /* No sense continuing if we can't use WinSock */
    } else {
          
      DialogBox (hInst, MAKEINTRESOURCE(AS_ECHO), NULL, Dlg_Main);
    
      /*---------------release WinSock DLL--------------*/
      nRet = WSACleanup();
      if (nRet == SOCKET_ERROR)
        WSAperror(WSAGetLastError(), "WSACleanup()", hInst);
    }    
        
    return msg.wParam;
} /* end WinMain() */

/*--------------------------------------------------------------------
 * Function: Dlg_Main()
 *
 * Description:
 *  Process dialog messages and asynchronous WinSock messages.
 */
BOOL CALLBACK Dlg_Main 
  (HWND hDlg,
   UINT msg,
   UINT wParam,
   LPARAM lParam)
{                      
    LPCONNDATA lpstSock;             /* Work Pointer */
    WORD WSAEvent, WSAErr;
    SOCKADDR_IN stRmtName;
    SOCKET hSock;
    BOOL bRet = FALSE;
    int  nRet, cbRcvd, cbSent;
   
    switch (msg) {
      case WSA_ASYNC:
        /*------------------------------------- 
         * Async notification message handlers 
         *------------------------------------*/ 
        hSock = (SOCKET)wParam;                 /* socket */
        WSAEvent = WSAGETSELECTEVENT (lParam);  /* extract event */
        WSAErr   = WSAGETSELECTERROR (lParam);  /* extract error */
        lpstSock = FindConn(hSock);             /* find our socket structure */

        /* Close connection on error (don't show error message in server */
        if (WSAErr && (hSock != hLstnSock))  {
          PostMessage (hWinMain, WSA_ASYNC,
              (WPARAM)hSock, WSAMAKESELECTREPLY(FD_CLOSE,0)); 
          break;
        }
        switch (WSAEvent) {
          case FD_READ:
            if (lpstSock) {
              /* Read data from socket and write it back */
              cbRcvd = lpstSock->iBytesRcvd;
              if (((INPUT_SIZE) - cbRcvd) > 0) {
                lpstSock->bReadPending = FALSE;
                nRet = RecvData(hSock, 
                  (LPSTR)&(lpstSock->achIOBuf[cbRcvd]), 
                  INPUT_SIZE-cbRcvd);
                lpstSock->iBytesRcvd += nRet;
                lpstSock->lByteCount += nRet;
                lByteCount += nRet;
                _ltoa(lByteCount, achTempBuf, 10);
                SetDlgItemText(hWinMain, IDC_BYTE_TOTAL, achTempBuf);
              } else {
                /* No buffer space now, so defer the net read */
                lpstSock->bReadPending = TRUE;
              }
            }
            /* Now write data back to client */
            cbSent = lpstSock->iBytesSent;
            lpstSock->iBytesSent += SendData(hSock,
              (LPSTR)&(lpstSock->achIOBuf[cbSent]),
              lpstSock->iBytesRcvd - cbSent);

            /* If we sent everything we received, reset counters */  
            if (lpstSock->iBytesSent == lpstSock->iBytesRcvd) {
              lpstSock->iBytesSent = 0;
              lpstSock->iBytesRcvd = 0;
            }
            break;
          case FD_WRITE:                                             
            /* Send data (may not be any to send initially) */
            if (lpstSock && lpstSock->iBytesRcvd) {
              cbSent = lpstSock->iBytesSent;
              lpstSock->iBytesSent += SendData(hSock,
                (LPSTR)&(lpstSock->achIOBuf[cbSent]),
                lpstSock->iBytesRcvd - cbSent);

              /* If we sent everything we received, reset counters */  
              if (lpstSock->iBytesSent == lpstSock->iBytesRcvd) {
                lpstSock->iBytesSent = 0;
                lpstSock->iBytesRcvd = 0;
              }
              
              /* If there's a read pending, then do it */
              if (lpstSock->bReadPending) {
                PostMessage (hWinMain, WSA_ASYNC,
                   (WPARAM)hSock, WSAMAKESELECTREPLY(FD_READ, 0));
              }
            }
            break;
          case FD_ACCEPT:
            /* Accept the incoming data connection request */
            hSock = AcceptConn(hLstnSock, &stRmtName);
            if (hSock != INVALID_SOCKET) {
              /* get a new socket structure */
              lpstSock = NewConn(hSock, &stRmtName);
              if (!lpstSock) {
                CloseConn(hSock, (LPSTR)0, INPUT_SIZE, hWinMain);
              } else {
                iActiveConns++;
                SetDlgItemInt(hWinMain, IDC_CONN_ACTIVE, iActiveConns, FALSE);
              }
            }
            break;
          case FD_CLOSE:                    /* Data connection closed */
            if (hSock != hLstnSock) {
              /* Read any remaining data and close connection */  
              CloseConn(hSock, (LPSTR)0, INPUT_SIZE, hWinMain);
              if (lpstSock) {
                DoStats(lpstSock->lByteCount, lpstSock->lStartTime, lpstSock);
                RemoveConn(lpstSock);
                iTotalConns++;
                SetDlgItemInt(hWinMain, IDC_CONN_TOTAL, iTotalConns, FALSE);
                iActiveConns--;
                SetDlgItemInt(hWinMain, IDC_CONN_ACTIVE, iActiveConns, FALSE);
              }
            }
            break;
          default:
             break;
        } /* end switch(WSAEvent) */
        break;

      case WM_COMMAND:
        switch (wParam) {
             
           case IDC_ABOUT:
             DialogBox (hInst, MAKEINTRESOURCE(IDD_ABOUT), hDlg, Dlg_About);
             break;

           case WM_DESTROY:
           case IDC_EXIT:
             /* Close listening socket */
             if (hLstnSock != INVALID_SOCKET)
               closesocket(hLstnSock);
             
             /* Close all active connections */
             for (lpstSock = lpstSockHead; 
                  lpstSock;
                  lpstSock=lpstSock->lpstNext) {
               CloseConn(lpstSock->hSock, (LPSTR)0, INPUT_SIZE, hWinMain);
               RemoveConn(lpstSock); 
             }
             /* Write final stats, and close logfile */
             if (hLogFile != HFILE_ERROR) {
               wsprintf (achTempBuf, 
                 "Final Totals: Bytes Received: %lu, Connections: %d\r\n",
                 lByteCount, iTotalConns);
               _lwrite (hLogFile, achTempBuf, strlen(achTempBuf));
               _lclose (hLogFile);
             }
             EndDialog(hDlg, msg);
             bRet = TRUE;
             break;
                       
           default:
              break;
         } /* end case WM_COMMAND: */
         break;
           
      case WM_INITDIALOG:
        hWinMain = hDlg;	/* save our main window handle */
        
        /* Get a socket listening */
        hLstnSock = InitLstnSock (IPPORT_ECHO,&stLclName,hWinMain,WSA_ASYNC);
        
        /* Get our local info for display */
        stLclName.sin_addr.s_addr = GetHostID ();
        gethostname (szHostName, MAXHOSTNAME);
        wsprintf(achTempBuf, "Local Host: %s (%s)",
            inet_ntoa(stLclName.sin_addr),
            szHostName[0] ? (LPSTR)szHostName : (LPSTR)"<unknown>");
        SetDlgItemText (hWinMain, IDC_LOCAL_HOST, achTempBuf);
                    
        /* Assign an icon to dialog box */
#ifndef WIN32
	SetClassWord(hDlg,GCW_HICON,
#else		
	SetClassLong(hDlg,GCL_HICON,
#endif
          (WORD)LoadIcon(hInst,MAKEINTRESOURCE(AS_ECHO)));

        /* Open logfile, if logging enabled */
        hLogFile = _lcreat (szLogFile, 0);
        if (hLogFile == HFILE_ERROR) { 
          MessageBox (hWinMain, "Unable to open logfile",
            "File Error", MB_OK | MB_ICONASTERISK);
        }
        /* Center dialog box */
        CenterWnd (hDlg, NULL, TRUE);
        break;
             
      default:
        break;
  } /* end switch (msg) */

  return (bRet);
} /* end Dlg_Main() */

/*---------------------------------------------------------------
 * Function: InitLstnSock()
 *
 * Description: Get a stream socket, and start listening for 
 *  incoming connection requests.
 */
BOOL InitLstnSock(int iLstnPort, PSOCKADDR_IN pstSockName, 
  HWND hWnd, u_int nAsyncMsg)
{
  int nRet;
  SOCKET hLstnSock;
  int nLen = SOCKADDR_LEN;
  
  /* Get a TCP socket to use for data connection listen */
  hLstnSock = socket (AF_INET, SOCK_STREAM, 0);
  if (hLstnSock == INVALID_SOCKET)  {
    WSAperror(WSAGetLastError(), "socket()", hInst);
  } else {
    /* Request async notification for most events */
    nRet = WSAAsyncSelect(hLstnSock, hWnd, nAsyncMsg, 
           (FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE));
    if (nRet == SOCKET_ERROR) {
      WSAperror(WSAGetLastError(), "WSAAsyncSelect()", hInst);
    } else {
                   
      /* Name the local socket with bind() */
      pstSockName->sin_family = PF_INET;
      pstSockName->sin_port   = (u_short) htons((u_short)iLstnPort);  
      nRet = bind(hLstnSock,(LPSOCKADDR)pstSockName,SOCKADDR_LEN);
      if (nRet == SOCKET_ERROR) {
	    WSAperror(WSAGetLastError(), "bind()", hInst);
      } else {

        /* Listen for incoming connection requests */
        nRet = listen(hLstnSock, 5);
        if (nRet == SOCKET_ERROR) {
          WSAperror(WSAGetLastError(), "listen()", hInst);
        }
      }
    }
    /* If we had an error then we have a problem.  Clean up */
    if (nRet == SOCKET_ERROR) {
	  closesocket(hLstnSock);
	  hLstnSock = INVALID_SOCKET;
    }
  }
  return (hLstnSock);
} /* end InitLstnSock() */

/*--------------------------------------------------------------
 * Function: AcceptConn()
 *
 * Description: Accept an incoming connection request (this is
 *  called in response to an FD_ACCEPT event notification).
 */
SOCKET AcceptConn(SOCKET hLstnSock, PSOCKADDR_IN pstName)
{
  SOCKET hNewSock;
  int nRet, nLen = SOCKADDR_LEN;
  
  hNewSock = accept (hLstnSock, (LPSOCKADDR)pstName, (LPINT)&nLen);
  if (hNewSock == SOCKET_ERROR) {
    int WSAErr = WSAGetLastError();
    if (WSAErr != WSAEWOULDBLOCK)
      WSAperror (WSAErr, "accept", hInst);
  } else if (bReAsync) {
    /* This SHOULD be unnecessary, since all new sockets are supposed
     *  to inherit properties of the listening socket (like all the
     *  asynch events registered but some WinSocks don't do this.
     * Request async notification for most events */
    nRet = WSAAsyncSelect(hNewSock, hWinMain, WSA_ASYNC, 
           (FD_READ | FD_WRITE | FD_CLOSE));
    if (nRet == SOCKET_ERROR) {
      WSAperror(WSAGetLastError(), "WSAAsyncSelect()", hInst);
    }
    /* Try to get lots of buffer space */
    GetBuf(hNewSock, INPUT_SIZE, SO_RCVBUF);
    GetBuf(hNewSock, INPUT_SIZE, SO_SNDBUF);
  }
  return (hNewSock);
} /* end AcceptConn() */

/*--------------------------------------------------------------
 * Function: SendData()
 *
 * Description: Send data received back to client that sent it.
 */
int SendData(SOCKET hSock, LPSTR lpOutBuf, int cbTotalToSend)
{
  int cbTotalSent  = 0;
  int cbLeftToSend = cbTotalToSend;
  int nRet, WSAErr;

  /* Send as much data as we can */
  while (cbLeftToSend > 0) {
  
    /* Send data to client */
    nRet = send (hSock, lpOutBuf+cbTotalSent, 
      cbLeftToSend < MTU_SIZE ? cbLeftToSend : MTU_SIZE, 0);

    if (nRet == SOCKET_ERROR) {
      WSAErr = WSAGetLastError();
      /* Display significant errors, then close connection */
      if (WSAErr != WSAEWOULDBLOCK) {
        WSAperror(WSAErr, (LPSTR)"send()", hInst);
        PostMessage(hWinMain, WSA_ASYNC, hSock, WSAMAKEASYNCREPLY(FD_CLOSE,0));
      }
      break;
    } else {
      /* Update byte counter, and display. */
      cbTotalSent += nRet;
    }
    /* calculate what's left to send */
    cbLeftToSend = cbTotalSent - cbTotalToSend; 
  }
  return (cbTotalSent);
} /* end SendData() */

/*--------------------------------------------------------------
 * Function: RecvData()
 *
 * Description: Receive data into buffer 
 */
int RecvData(SOCKET hSock, LPSTR lpInBuf, int cbTotalToRecv)
{
  int cbTotalRcvd = 0;
  int cbLeftToRecv = cbTotalToRecv;
  int nRet=0, WSAErr;

  /* Read as much as we can buffer from client */
  while (cbLeftToRecv > 0) {

    nRet = recv (hSock,lpInBuf+cbTotalRcvd, cbLeftToRecv, 0);
    if (nRet == SOCKET_ERROR) {
      WSAErr = WSAGetLastError();
      /* Display significant errors */
      if (WSAErr != WSAEWOULDBLOCK) {
        WSAperror(WSAErr, (LPSTR)"recv()", hInst);
        PostMessage(hWinMain, WSA_ASYNC, hSock, WSAMAKEASYNCREPLY(FD_CLOSE,0));
      }
      /* exit recv() loop on any error */
      break;
    } else if (nRet == 0) { /* Other side closed socket */
      /* quit if server closed connection */
      break;
    } else {
      /* Update byte counter */
      cbTotalRcvd += nRet;
    }
    cbLeftToRecv = cbTotalToRecv - cbTotalRcvd;
  }
  return (cbTotalRcvd);
} /* end RecvData() */

/*--------------------------------------------------------------
 * Function: DoStats()
 *
 * Description: Display the connection data rate statistics
 *  (called after a connection closes in response to FD_CLOSE).
 */
void DoStats (long lByteCount, long lStartTime, LPCONNDATA lpstSock) {
  LONG dByteRate;
  LONG lMSecs;

  /* Calculate data transfer rate, and display */
  lMSecs = (LONG) GetTickCount() - lStartTime;
  if (lMSecs <= 55)
    lMSecs = 27;  /* about half of 55Msec PC clock resolution */
              
  if (lByteCount > 0L) {
    if (lpstSock) {
      wsprintf (achTempBuf,
      "<<< socket: %d disconnected from %s\r\n", 
        lpstSock->hSock, 
        inet_ntoa(lpstSock->stRmtName.sin_addr));
      _lwrite(hLogFile, achTempBuf, strlen(achTempBuf));
    }
    dByteRate = (lByteCount/lMSecs); /* data rate (bytes/Msec) */
    wsprintf (achTempBuf,
      "%ld bytes in %ld.%ld seconds (%ld.%ld Kbytes/sec)\n",
      lByteCount, 
      lMSecs/1000, lMSecs%1000, 
      (dByteRate*1000)/1024, (dByteRate*1000)%1024);
    SetDlgItemText (hWinMain, IDC_DATA_RATE, achTempBuf);
    if (hLogFile != HFILE_ERROR)
      _lwrite (hLogFile, achTempBuf, strlen(achTempBuf));
  }
} /* end DoStats() */            

/*---------------------------------------------------------------
 * Function:NewConn()
 *
 * Description: Create a new socket structure and put in list
 */
LPCONNDATA NewConn (SOCKET hSock, PSOCKADDR_IN pstRmtName) {
  int nAddrSize = sizeof(SOCKADDR);
  LPCONNDATA lpstSockTmp, lpstSock = (LPCONNDATA)0;
  HLOCAL hConnData;

  /* Allocate memory for the new socket structure */
  hConnData = LocalAlloc (LMEM_ZEROINIT, sizeof(CONNDATA));
  
  if (hConnData != 0) {
    /* Lock it down and link it into the list */
    lpstSock = LocalLock(hConnData);
    
    if (!lpstSockHead) {
      lpstSockHead = lpstSock;
    } else {
      for (lpstSockTmp = lpstSockHead; 
           lpstSockTmp && lpstSockTmp->lpstNext; 
           lpstSockTmp = lpstSockTmp->lpstNext);
      lpstSockTmp->lpstNext = lpstSock;
    }
  
    /* Initialize socket structure */
    lpstSock->hSock = hSock;
    _fmemcpy ((LPSTR)&(lpstSock->stRmtName), 
              (LPSTR)pstRmtName, sizeof(SOCKADDR));
    lpstSock->lStartTime = GetTickCount();
        
    /* Log the new connection */
    if (hLogFile != HFILE_ERROR) {
      wsprintf(achTempBuf, 
        ">>> socket: %d connected from %s\r\n", hSock, 
        inet_ntoa(lpstSock->stRmtName.sin_addr));
      _lwrite(hLogFile, achTempBuf, strlen(achTempBuf));
    }
  } else {
    MessageBox (hWinMain, "Unable allocate memory for connection",
      "LocalAlloc() Error", MB_OK | MB_ICONASTERISK);
  }
  return (lpstSock);
} /* end NewConn() */  

/*---------------------------------------------------------------
 * Function: FindConn()
 *
 * Description: Find socket structure for connection
 */
LPCONNDATA FindConn (SOCKET hSock) {
  LPCONNDATA lpstSockTmp;
  
  for (lpstSockTmp = lpstSockHead; 
       lpstSockTmp;
       lpstSockTmp = lpstSockTmp->lpstNext) {
    if (lpstSockTmp->hSock == hSock)
      break;
  }
       
  return (lpstSockTmp);
} /* end FindConn() */  

/*---------------------------------------------------------------
 * Function: RemoveConn()
 *
 * Description: Free the memory for socket structure
 */
void RemoveConn (LPCONNDATA lpstSock) {
  LPCONNDATA lpstSockTmp;
  HLOCAL hSock;

  if (lpstSock == lpstSockHead) {
    lpstSockHead = lpstSock->lpstNext;
  } else {  
    for (lpstSockTmp = lpstSockHead; 
         lpstSockTmp;
         lpstSockTmp = lpstSockTmp->lpstNext) {
      if (lpstSockTmp->lpstNext == lpstSock)
        lpstSockTmp->lpstNext = lpstSock->lpstNext;
    }     
  }
  hSock = LocalHandle(lpstSock);
  LocalUnlock (hSock);
  LocalFree (hSock);
} /* end RemoveConn() */  

