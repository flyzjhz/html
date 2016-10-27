/*---------------------------------------------------------------------
 *
 * Program: WSASIMPL.DLL  Simplified WinSock API (for TCP Clients)
 *
 * filename: wsasimpl.c
 *
 * copyright by Bob Quinn, 1995
 *   
 *  Description:
 *    This DLL provides an "encapsulated WinSock API," as described in
 *    Chapter 12 of _Windows Sockets Network Programming_.  It uses
 *    a helper task. a hidden window and asynchronous operation mode
 *    to provide basic functionality for TCP client applications.
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
#include <string.h>   /* for _fmemcpy() */
#include <stdlib.h>   /* for atoi() */
#include "..\winsockx.h"

#define TIMEOUT_ID  WM_USER+1

/*------- important data structures ------*/
typedef struct TaskData {
  HTASK  hTask;                   /* Task ID: primary key */
  int    nRefCount;               /* Number of sockets owned by task */
  struct TaskData *lpstNext;      /* Pointer to next entry in linked list */
} TASKDATA, *PTASKDATA, FAR *LPTASKDATA;

typedef struct ConnData {
  SOCKET hSock;                   /* Conection socket */
  LPTASKDATA lpstTask;            /* Pointer to Task structure */
  HWND   hwnd;                    /* Handle of subclassed window */
  SOCKADDR_IN stRmtName;          /* Remote host address & port */
  int   nTimeout;                 /* Timeout (in millisecs) */
  DWORD lpfnWndProc;              /* Window procedure (before subclassed) */
  struct ConnData *lpstNext;      /* Pointer to next entry in linked list */
} CONNDATA, *PCONNDATA, FAR *LPCONNDATA;

/*-------------- global data -------------*/
char szAppName[] = "wsasimpl";

HWND hWinMain;
HINSTANCE hInst;
 
WSADATA stWSAData;

LPCONNDATA lpstConnHead = 0L;     /* Head of connection data list */
LPTASKDATA lpstTaskHead = 0L;     /* Head of task data list */

/*-------- exported function prototypes ---------*/
int  WINAPI LibMain (HANDLE, WORD, WORD, LPSTR);
LONG CALLBACK SubclassProc (HWND, UINT, WPARAM, LPARAM);
SOCKET WINAPI ConnectTCP(LPSTR, LPSTR);
int WINAPI SendData(SOCKET, LPSTR, int);
int WINAPI RecvData(SOCKET, LPSTR, int, int);
int WINAPI CloseTCP(SOCKET, LPSTR, int);

/*-------- internal function prototypes ---------*/
int DoSend(SOCKET, LPSTR, int, LPCONNDATA);
int DoRecv(SOCKET, LPSTR, int, LPCONNDATA);
LPCONNDATA NewConn (SOCKET, PSOCKADDR_IN);
LPCONNDATA FindConn (SOCKET, HWND);
void      RemoveConn (LPCONNDATA);
LPTASKDATA NewTask (HTASK);
LPTASKDATA FindTask (HTASK);
void      RemoveTask (LPTASKDATA);
u_short GetPort (LPSTR);

/*-------------------------------------------------------------------- 
 *  Function: LibMain()
 *
 *  Description: DLL entry point (we don't have much to do here)
 */
int PASCAL LibMain
  (HANDLE hInstance,
   WORD   wDataSeg,
   WORD   wHeapSize,
   LPSTR  lpszCmdLine)
{
    
    lpszCmdLine = lpszCmdLine; /* avoid warnings */
    wDataSeg    = wDataSeg;
    wHeapSize   = wHeapSize;

    hInst = hInstance;	/* save instance handle */
//    GlobalLock(hInst);

    return (1);
} /* end LibMain() */

/*--------------------------------------------------------------------
 * Function: SubclassProc()
 *
 * Description: process messages bound for calling application to
 *  filter out "reentrant" mouse and keyboard messages while a
 *  blocking network operation is pending.
 */
LONG CALLBACK SubclassProc 
  (HWND hwnd,
   UINT msg,
   WPARAM wParam,
   LPARAM lParam)
{
    LPCONNDATA lpstConn;             /* Work Pointer */
   
    lpstConn = FindConn(0, hwnd);    /* find our socket structure */

    switch (msg) {
      case WM_QUIT:
        /* Close this connection */
        if (lpstConn) {
          CloseTCP(lpstConn->hSock, (LPSTR)0, INPUT_SIZE);
          RemoveConn(lpstConn); 
          /* Release Timer (if it's active) */
          if (lpstConn->nTimeout)
            KillTimer(hwnd, TIMEOUT_ID);
        }
        break;
           
      case WM_CLOSE:
      case WM_TIMER:
        /* If Timeout or Close request, cancel pending */
        if(lpstConn && WSAIsBlocking())
            WSACancelBlockingCall();
        break;
      
      case WM_KEYDOWN:
      case WM_KEYUP:
      case WM_LBUTTONDBLCLK:
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_MBUTTONDBLCLK:
      case WM_MBUTTONDOWN:
      case WM_MBUTTONUP:
      case WM_MOUSEACTIVATE:
      case WM_MOUSEMOVE:
      case WM_NCHITTEST:
      case WM_NCLBUTTONDBLCLK:
      case WM_NCLBUTTONDOWN:
      case WM_NCLBUTTONUP:
      case WM_NCMBUTTONDBLCLK:
      case WM_NCMBUTTONDOWN:
      case WM_NCMBUTTONUP:
      case WM_NCMOUSEMOVE:
      case WM_NCRBUTTONDBLCLK:
      case WM_NCRBUTTONDOWN:
      case WM_NCRBUTTONUP:
      case WM_NEXTDLGCTL:
      case WM_RBUTTONDBLCLK:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_SYSCHAR:
      case WM_SYSDEADCHAR:
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
        /* Eat all mouse and keyboard messages */
        return (0L);
        
    default:
        break;
  } /* end switch (msg) */

  if (lpstConn) {  
    /* Let original (pre-subclass) window handler process message */
    return (CallWindowProc ((WNDPROC)(lpstConn->lpfnWndProc),
         hwnd, msg, wParam, lParam));
  } else {
    return (0L);  /* this should never occur */
  }
} /* end SubClassProc() */

/*---------------------------------------------------------------
 * Function: ConnectTCP()
 *
 * Description: get a TCP socket and connect to server (along with
 *  other maintenance stuff, like subclassing window, and registering
 *  task).
 */
SOCKET WINAPI ConnectTCP(LPSTR szDestination, LPSTR szService) 
{
  int nRet;
  HTASK hTask;
  SOCKET hSock;
  LPTASKDATA lpstTask;
  LPCONNDATA lpstConn;
  SOCKADDR_IN stRmtName;

#ifndef WIN32                           
  hTask = GetCurrentTask();     /* Task handle: for our records */
#else
  hTask = GetCurrentProcess();  /* or Process handle (if 32-bit) */
#endif
  lpstTask = FindTask (hTask);
  if (!lpstTask) {
    /* If task isn't registered, then register it (call WSAStartup()) */
    lpstTask = NewTask(hTask);
  }
  if (lpstTask) {
    /* Get a TCP socket */
    hSock = socket (AF_INET, SOCK_STREAM, 0);
    if (hSock == INVALID_SOCKET)  {
      WSAperror(WSAGetLastError(), "socket()", hInst);
    } else {
      /* Get destination address */
      stRmtName.sin_addr.s_addr = GetAddr(szDestination);
      
      if (stRmtName.sin_addr.s_addr != INADDR_NONE) {
        /* Get destination port number */
        stRmtName.sin_port = GetPort(szService);
       
        if (stRmtName.sin_port) {
          /* Create a new socket structure */
          lpstConn = NewConn(hSock, &stRmtName);    

          if (lpstConn) {
            /* Subclass the active window passed 
             *  NOTE: This reveals one limitation in our API.  This is the 
             *  same window we'll subclass during sends and receives, so
             *  we won't capture user I/O if application sends calls the
             *  SendData() or RecvData() from a different window. */
            lpstConn->lpstTask = lpstTask;
            lpstConn->hwnd = GetActiveWindow();
            lpstConn->lpfnWndProc = GetWindowLong(lpstConn->hwnd,GWL_WNDPROC);
            SetWindowLong (lpstConn->hwnd, GWL_WNDPROC, (DWORD)SubclassProc);
          
            /* Initiate non-blocking connect to server */
            stRmtName.sin_family = PF_INET;
            nRet = connect(hSock,(LPSOCKADDR)&stRmtName,SOCKADDR_LEN);
        
            /* Unsubclass active window now that we're done blocking */
            SetWindowLong(lpstConn->hwnd,GWL_WNDPROC,(DWORD)lpstConn->lpfnWndProc);

            if (nRet == SOCKET_ERROR) {
	          int WSAErr = WSAGetLastError();

  	          if (WSAErr != WSAEINTR) {
	            /* Display all errors except "operation interrupted"*/
	            WSAperror(WSAErr, "connect()", hInst);
                RemoveConn(lpstConn);
                lpstTask = 0L;
	            closesocket(hSock);
	            hSock = INVALID_SOCKET;
	          }
            }
          } else {
            /* Can't create a connection structure */
            closesocket(hSock);
            hSock = INVALID_SOCKET;
          }
        } else {
          /* Can't resolve destination port number */
          closesocket(hSock);
          hSock = INVALID_SOCKET;
        }
      } else {
        /* Can't resolve destination address */
        closesocket(hSock);
        hSock = INVALID_SOCKET;
      }
    }
    /* If we failed, we need to clean up */
    if (hSock == INVALID_SOCKET && lpstTask) {
      RemoveTask(lpstTask);
    }
  }
  return (hSock);
} /* end ConnectTCP() */

/*--------------------------------------------------------------
 * Function: SendData()
 *
 * Description: Send data amount requested from buffer passed
 */
int WINAPI SendData(SOCKET hSock, LPSTR lpOutBuf, int cbTotalToSend)
{
  LPCONNDATA lpstConn;
  int cbTotalSent = 0, cbSent;
  int nRet = SOCKET_ERROR;   /* assume error */
  
  lpstConn = FindConn(hSock, 0);
  if (!lpstConn) {
    /* Socket not found, so it's not valid */
    WSASetLastError(WSAENOTSOCK);
    
  } else {
    /* Subclass the window provided at connnect to filter message traffic */
    SetWindowLong (lpstConn->hwnd, GWL_WNDPROC, (DWORD)SubclassProc);
      
    while  (((cbTotalToSend - cbTotalSent) > 0) && 
	    (lpstConn->hSock != INVALID_SOCKET)) {
      cbSent = DoSend(hSock,
                      lpOutBuf+cbTotalSent,
                      cbTotalToSend - cbTotalSent,
                      lpstConn);
      if (cbSent != SOCKET_ERROR) {
        /* Tally and Quit the loop if we've sent amount requested */
        cbTotalSent += cbSent;
        if ((cbTotalToSend - cbTotalSent) <= 0)
          break;
      } else {
        /* If send failed, return an error */
        cbTotalSent = SOCKET_ERROR;
      }
    }
    /* Unsubclass active window before we leave */
    SetWindowLong(lpstConn->hwnd, GWL_WNDPROC, (long)lpstConn->lpfnWndProc);
  }
  return (cbTotalSent);
} /* end SendData() */

/*--------------------------------------------------------------
 * Function: DoSend()
 *
 * Description: Send data. We call this function from SendData(),
 *  or in response to FD_WRITE asynchronous notification.
 */
int DoSend(SOCKET hSock, LPSTR lpOutBuf, int cbTotalToSend, 
  LPCONNDATA lpstConn)
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
      /* Display all errors except "operation interrupted" */
      if (WSAErr != WSAEINTR) {
        /*  unsubclass first so user can respond to error) */
        SetWindowLong(lpstConn->hwnd, GWL_WNDPROC,
	   (DWORD)lpstConn->lpfnWndProc);
        WSAperror(WSAErr, (LPSTR)"send()", hInst);
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
} /* end DoSend() */

/*--------------------------------------------------------------
 * Function: RecvData()
 *
 * Description: Recieve data amount requested into buffer passed
 */
int WINAPI RecvData(SOCKET hSock, LPSTR lpInBuf, int cbTotalToRecv, int nTimeout)
{
  LPCONNDATA lpstConn;
  int cbTotalRcvd = 0, cbRcvd;
  int nRet = SOCKET_ERROR;   /* assume error */
  
  lpstConn = FindConn(hSock, 0);
  if (!lpstConn) {
    /* Socket not found, so it's not valid */
    WSASetLastError(WSAENOTSOCK);
    
  } else {
    /* Subclass the active window to filter message traffic */
    SetWindowLong (lpstConn->hwnd, GWL_WNDPROC, (DWORD)SubclassProc);
    
    /* Set a timer, if requested */
    if (nTimeout) {
      lpstConn->nTimeout = nTimeout;
      SetTimer(hWinMain, TIMEOUT_ID, nTimeout, 0L);
    }
      
    while  (((cbTotalToRecv - cbTotalRcvd) > 0) &&
	    (lpstConn->hSock != INVALID_SOCKET)) {
      cbRcvd = DoRecv(hSock,
                      lpInBuf+cbTotalRcvd,
                      cbTotalToRecv - cbTotalRcvd,
                      lpstConn);
      if (cbRcvd != SOCKET_ERROR) {
        /* Tally and Quit if we've received amount requested */
        cbTotalRcvd += cbRcvd;
        if ((cbTotalToRecv - cbTotalRcvd) <= 0) {
          if (lpstConn->nTimeout)
            /* Release timer, if there is one */
            KillTimer (lpstConn->hwnd, TIMEOUT_ID);
          break;
        }
        if (lpstConn->nTimeout) {
          /* Reset timer, if there is one */
          SetTimer(hWinMain, TIMEOUT_ID, lpstConn->nTimeout, 0L);
        }
      } else {
        /* If receive failed, return an error */
        cbTotalRcvd = SOCKET_ERROR;
      }
    }
    /* Unsubclass active window before we leave */
    SetWindowLong(lpstConn->hwnd, GWL_WNDPROC, (long)lpstConn->lpfnWndProc);
    lpstConn->nTimeout = 0;  /* reset timer */
  }
  return (cbTotalRcvd);
} /* end RecvData() */

/*--------------------------------------------------------------
 * Function: DoRecv()
 *
 * Description: Receive data into buffer.  We call this function
 *  in response to FD_READ asynchronous notification. 
 */
int DoRecv(SOCKET hSock, LPSTR lpInBuf, int cbTotalToRecv,
  LPCONNDATA lpstConn)
{
  int cbTotalRcvd = 0;
  int cbLeftToRecv = cbTotalToRecv;
  int nRet=0, WSAErr;

  /* Read as much as we can buffer from client */
  while (cbLeftToRecv > 0) {

    nRet = recv (hSock,lpInBuf+cbTotalRcvd, cbLeftToRecv, 0);
    if (nRet == SOCKET_ERROR) {
      WSAErr = WSAGetLastError();
      /* Display all errors except "operation interrupted" */
      if (WSAErr != WSAEINTR) {
        WSAperror(WSAErr, (LPSTR)"recv()", hInst);
        /*  unsubclass first so user can respond to error) */
        SetWindowLong(lpstConn->hwnd,GWL_WNDPROC,
	   (DWORD)lpstConn->lpfnWndProc);
      }
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
} /* end DoRecv() */

/*--------------------------------------------------------------
 * Function: CloseTCP()
 *
 * Description: Do a graceful close of a TCP connection using 
 *  the robust algorithm of "half-closing" with shutdown(how=1),
 *  then loop on recv() to read remaining data until it fails,
 *  or returns a zero, then call closesocket().
 */
int WINAPI CloseTCP(SOCKET hSock, LPSTR lpInBuf, int len)
{
  int nRet = SOCKET_ERROR, cbBytesDone=0;
  LPCONNDATA lpstConn;
 
  lpstConn = FindConn(hSock, 0);
  if (!lpstConn) {
    /* Socket not found, so it's not valid */
    WSASetLastError(WSAENOTSOCK);
  } else {
    if (WSAIsBlocking()) {
      /* Can't close socket now since blocking operation pending,
       *  so just cancel the blocking operation and we'll close
       *  connection when pending operation fails with WSAEINTR */
       WSACancelBlockingCall();
    } else {
      /* Signal the end is near */
      lpstConn->hSock = INVALID_SOCKET;
         
      /* Half-close the connection to close neatly (we ignore the error here
       *  since some WinSocks fail with WSAEINVAL if they've recieved a RESET
       *  on the socket before the call to shutdown(). */
      nRet = shutdown (hSock, 1);
    
      /* Read and discard remaining data (until EOF or any error) */
      nRet = 1;
      while (nRet && (nRet != SOCKET_ERROR)) {
          nRet = recv (hSock, lpInBuf, len-cbBytesDone, 0);
          if (nRet > 0)
            cbBytesDone += nRet;
      }
    
      /* close the socket, and ignore any error (since we can't do much 
       *  about them anyway */
      nRet = closesocket (hSock);
    }
    RemoveConn(lpstConn);
  }
  return (nRet);
} /* end CloseTCP() */

/*---------------------------------------------------------------
 * Function:NewConn()
 *
 * Description: Create a new socket structure and put in list
 */
LPCONNDATA NewConn (SOCKET hSock,PSOCKADDR_IN lpstRmtName) {
  int nAddrSize = sizeof(SOCKADDR);
  LPCONNDATA lpstConnTmp;
  LPCONNDATA lpstConn = (LPCONNDATA)0;
  HLOCAL hConnData;

  /* Allocate memory for the new socket structure */
  hConnData = LocalAlloc (LMEM_ZEROINIT, sizeof(CONNDATA));
  
  if (hConnData) {
    /* Lock it down and link it into the list */
    lpstConn = (LPCONNDATA) LocalLock(hConnData);
    
    if (!lpstConnHead) {
      lpstConnHead = lpstConn;// BQ NOTE: This Doesn't Work for some reason!
    } else {
      for (lpstConnTmp = lpstConnHead; 
           lpstConnTmp && lpstConnTmp->lpstNext; 
           lpstConnTmp = lpstConnTmp->lpstNext);
      lpstConnTmp->lpstNext = lpstConn;
    }
    /* Initialize socket structure */
    lpstConn->hSock = hSock;
    _fmemcpy ((LPSTR)&(lpstConn->stRmtName), 
              (LPSTR)lpstRmtName, sizeof(SOCKADDR));
  }
  return (lpstConn);
} /* end NewConn() */  

/*---------------------------------------------------------------
 * Function: FindConn()
 *
 * Description: Find socket structure for connection using 
 *  either socket or window as search key.
 */
LPCONNDATA FindConn (SOCKET hSock, HWND hwnd) {
  LPCONNDATA lpstConnTmp;
  
  for (lpstConnTmp = lpstConnHead; 
       lpstConnTmp;     
       lpstConnTmp = lpstConnTmp->lpstNext) {
    if (hSock) {
      if (lpstConnTmp->hSock == hSock)
        break;
    } else if (lpstConnTmp->hwnd == hwnd) {
      break;
    }
  }
  return (lpstConnTmp);
} /* end FindConn() */  

/*---------------------------------------------------------------
 * Function: RemoveConn()
 *
 * Description: Free the memory for socket structure
 */
void RemoveConn (LPCONNDATA lpstConn) {
  LPCONNDATA lpstConnTmp;
  HLOCAL hConnTmp;
  
  if (lpstConn == lpstConnHead) {
    lpstConnHead = lpstConn->lpstNext;
  } else {  
    for (lpstConnTmp = lpstConnHead; 
         lpstConnTmp;
         lpstConnTmp = lpstConnTmp->lpstNext) {
      if (lpstConnTmp->lpstNext == lpstConn) {
        lpstConnTmp->lpstNext = lpstConn->lpstNext;
      }
    }     
  }
  RemoveTask (lpstConn->lpstTask);
  hConnTmp = LocalHandle((void NEAR*)lpstConn);
  LocalUnlock (hConnTmp);
  LocalFree (hConnTmp);
} /* end RemoveConn() */  

/*---------------------------------------------------------------
 * Function: NewTask()
 *
 * Description: Register task with WinSock DLL by calling 
 *  WSAStartup(), and create a new Task structure
 */
LPTASKDATA NewTask (HTASK hTask)
{ 
  HANDLE hTaskData;
  LPTASKDATA lpstTask = (LPTASKDATA)0L;
  int nRet;

  /* Register task with WinSock DLL */
  nRet = WSAStartup(WSA_VERSION, &stWSAData);
  if (nRet != 0) {
    WSAperror(nRet, "WSAStartup()", hInst);
  } else {
    /* Allocate memory for a window structure */
    hTaskData = 
      LocalAlloc(LMEM_MOVEABLE|LMEM_ZEROINIT, sizeof(TASKDATA));
    if (hTaskData) {
  
      /* Convert it to a pointer */
      lpstTask = (LPTASKDATA) LocalLock (hTaskData);
      if (lpstTask) {
      
        /* Initialize structure */
        lpstTask->hTask = hTask;
        lpstTask->nRefCount = 1;
      
        /* Link this new record into our linked list */
        if (!lpstTaskHead) {
          lpstTaskHead = lpstTask;
        } else {
          LPTASKDATA lpstTaskTmp;
          for (lpstTaskTmp = lpstTaskHead;
               lpstTaskTmp->lpstNext;
               lpstTaskTmp = lpstTaskTmp->lpstNext);
          lpstTaskTmp->lpstNext = lpstTask;
        }
      } else {
        /* Set error to indicate memory problems, and free memory */
        WSASetLastError(WSAENOBUFS);
        LocalFree(hTaskData);
      }
    } else {
      /* Set error to indicate we couldn't allocate memory */
      WSASetLastError(WSAENOBUFS);
    }
  }
  return (lpstTask);
} /* end NewTask() */

/*---------------------------------------------------------------
 * Function: FindTask()
 *
 * Description: Find Task structure using task handle as key.
 */
LPTASKDATA FindTask (HTASK hTask) {
  LPTASKDATA lpstTaskTmp;
  
  for (lpstTaskTmp = lpstTaskHead; 
       lpstTaskTmp;     
       lpstTaskTmp = lpstTaskTmp->lpstNext) {
    if (lpstTaskTmp->hTask == hTask)
      break;
  }
  return (lpstTaskTmp);
} /* end FindTask() */  

/*---------------------------------------------------------------
 * Function: RemoveTask()
 *
 * Description: Decrement the task reference count, and free 
 *  the memory for task structure when ref count is zero.
 */
void RemoveTask (LPTASKDATA lpstTask) {
  LPTASKDATA lpstTaskTmp;
  HLOCAL hTaskTmp;
  
  lpstTask->nRefCount--;
  if (lpstTask->nRefCount <= 0) {
    /* Reference count is zero, so free the task structure */
    if (lpstTask == lpstTaskHead) {
      lpstTaskHead = lpstTask->lpstNext;
    } else {  
      for (lpstTaskTmp = lpstTaskHead; 
           lpstTaskTmp;
           lpstTaskTmp = lpstTaskTmp->lpstNext) {
        if (lpstTaskTmp->lpstNext == lpstTask)
          lpstTaskTmp->lpstNext = lpstTask->lpstNext;
      }     
    }
    hTaskTmp = LocalHandle((void NEAR*)lpstTask);
    LocalUnlock (hTaskTmp);
    LocalFree (hTaskTmp);
    
    /* Lastly, call WSACleanup() to deregister task with WinSock */
    WSACleanup();
  }
} /* end RemoveTask() */  

#if 0
/*---------------------------------------------------------------
 * Function: WSAperror()
 *
 * Description:
 */
void WSAperror (int WSAErr, LPSTR szFuncName, HANDLE hInst) 
{
    static char achErrBuf [ERR_SIZE];	/* buffer for errors */
    static char achErrMsg [ERR_SIZE/2];

    WSAErrStr (WSAErr, (LPSTR)achErrMsg, hInst);
    
	wsprintf (achErrBuf, "%s failed,%-40c\n\n%s",
	  (LPSTR)szFuncName,' ',(LPSTR)achErrMsg);
            
	/* Display error message as is (even if incomplete) */
    MessageBox (GetActiveWindow(), (LPSTR)achErrBuf, (LPSTR)"Error", 
    			MB_OK | MB_ICONHAND);
    return;
}  /* end WSAperror() */

/*-----------------------------------------------------------
 * Function: WSAErrStr()
 *
 * Description: Given a WinSock error value, return error string
 *  NOTE: This function requires an active window to work if the
 *  instance handle is not passed.
 */
int WSAErrStr (int WSAErr, LPSTR lpErrBuf, HANDLE hInst) {
    int err_len=0;
    HWND hwnd;
    
    if (!hInst) {
      hwnd  = GetActiveWindow();
#ifndef WIN32
      hInst = GetWindowWord(hwnd, GWW_HINSTANCE);
#else
      hInst = (HANDLE) GetWindowLong(hwnd, GWL_HINSTANCE);
#endif
    }
    
    if (WSAErr == 0)           /* If error passed is 0, use the */
      WSAErr = WSABASEERR;     /*  base resource file number */
            
    if (WSAErr >= WSABASEERR)  /* Valid Error code? */
	  /* get error string from the table in the Resource file */
      err_len = LoadString(hInst, WSAErr, (LPSTR)lpErrBuf, ERR_SIZE/2);
    
    return (err_len);
}  /* end GetWSAErrStr() */

/*-----------------------------------------------------------
 * Function: GetAddr()
 *
 * Description: Given a string, it will return an IP address.
 *   - first it tries to convert the string directly
 *   - if that fails, it tries o resolve it as a hostname
 *
 * WARNING: gethostbyname() is a blocking function
 */
u_long GetAddr (LPSTR szHost) {
  LPHOSTENT lpstHost;
  u_long lAddr = INADDR_ANY;
  
  /* check that we have a string */
  if (*szHost) {
  
    /* check for a dotted-IP address string */
    lAddr = inet_addr (szHost);
  
    /* If not an address, then try to resolve it as a hostname */
    if ((lAddr == INADDR_NONE) &&
        (_fstrcmp (szHost, "255.255.255.255"))) {
      
      lpstHost = gethostbyname(szHost);
      if (lpstHost) {  /* success */
        lAddr = *((u_long FAR *) (lpstHost->h_addr));
      } else {  
        lAddr = INADDR_ANY;   /* failure */
      }
    }
  }
  return (lAddr); 
} /* end GetAddr() */

/*---------------------------------------------------------------
 * Function: GetPort()
 *
 * Description: Return a port number (in network order) from a 
 *  string.  May involve converting from ASCII to integer, or 
 *  resolving as service name.
 *
 * NOTE: This function is limited in several respects:
 *  1) it assumes the service name will not begin with an integer, 
 *     although it *is* possible.
 *  2) it assumes that the port number will be the same for both
 *     tcp & udp (it doesn't specify a protocol in getservbyname()).
 *  3) if it fails, there must be an active window to allow
 *     WSAperror() to display the error text.
 */
u_short GetPort (LPSTR szService)
{
  u_short nPort = 0;  /* Port 0 is invalid */
  LPSERVENT lpServent;
  char c;
  
  c = *szService;
  if ((c>='1') && (c<='9')) {
    /* Convert ASCII to integer, and put in network order */
    nPort = htons((u_short)atoi (szService));
  } else {
    /* Resolve service name to a port number */
    lpServent = getservbyname(szService, 0);
    if (!lpServent) {
      WSAperror (WSAGetLastError(), "getservbyname()", 0);
    } else {
      nPort = lpServent->s_port;
    }
  }
  return (nPort);
} /* end GetPort() */           
#endif
