/*---------------------------------------------------------------------
 *
 *  Program: MULTITST.EXE  Optional WinSock features test program
 *
 *  filename: multitst.c
 *
 *  copyright by Bob Quinn, 1995
 *
 *  Description: this program let's you test the optional features multicast, 
 *   raw sockets and IP_TTL socket option in a WinSock implementation.
 *
 *  Specifically, it tests whether a WinSock DLL supports Steve Deering's 
 *  multicast API, as described in his document "IP Multicast Extensions 
 *  for 4.3BSD UNIX and related systems" that acommpanied RFC-1054
 *  (subsequently obsoleted by RFC-1112 "Host Extensions for IP Multicasting")
 *  that describes the mechanics of multicasting, and in particular the
 *  role that Internet Group Management Protcol (IGMP) plays.
 *
 *  It also calls socket() with type=SOCK_RAW and protocol=IPROTO_ICMP
 *  to attempt a ping (ICMP echo request and reply).  If you select the
 *  traceroute option, it will try to call setsockopt() with the
 *  level=IPPROTO_IP and option=IP_TTL (value 4) to set the IP "time to
 *  live" field (for traceroute).
 * .
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

#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "resource.h"
#include "..\winsockx.h"
#include "..\wsa_xtra.h"

/* default multicast and destination port number to use */
#define DESTINATION_MCAST  "234.5.6.7"
#define DESTINATION_PORT  4567

#define USER_PARAMETER_VALUES 99
#define MCBUFSIZE 1024

/* display buffer (8K) */
#define DISP_LINE_LEN  128
#define DISP_NUM_LINES 64
#define DISP_BUFSIZE   DISP_LINE_LEN * DISP_NUM_LINES

/* externally defined functions */
extern SOCKET icmp_open (void);
extern int    icmp_close (SOCKET);
extern u_long icmp_sendto (SOCKET, HWND, LPSOCKADDR_IN, int, int, int);
extern u_long icmp_recvfrom (SOCKET, LPINT, LPINT, LPSOCKADDR_IN);
extern int    set_ttl (SOCKET, int);

/* internally defined public functions */
LRESULT CALLBACK MainWinProc(HWND, UINT, WPARAM, LPARAM);
BOOL FAR PASCAL BindDlgProc (HWND, UINT,  UINT, LPARAM);
BOOL FAR PASCAL SendDlgProc (HWND, UINT,  UINT, LPARAM);
BOOL FAR PASCAL OptionDlgProc (HWND, UINT,  UINT, LPARAM);
BOOL FAR PASCAL PingDlgProc (HWND, UINT, UINT, LPARAM);
void WSAErrMsg (LPSTR);

void InitWndData(HWND);
void UpdateDispBuf(HWND, LPSTR);
void UpdateWnd(HWND);

/* private data */
static HINSTANCE hInst;
static HWND      hwnd;

char  szAppName[] = "Multitst";

/* global data */ 
HWND hDeskTop;
RECT rcDeskTop;
int nDeskTopWidth, nXCenter, nDeskTopHeight, nYCenter;
int nMenuHeight, nCaptionHeight, nScrollWidth, nScrollHeight;
int nCharWidth, nCharHeight;
int nLinesToDisplay;
HGLOBAL hDispBuf;
LPSTR  lpDispBuf;
int nFirstLine=0;
int nLastLine=0;
int WSAInitFailed;
BOOL bBSD_OptNames=TRUE;  
char strDestMulti[MAXHOSTNAME] = {DESTINATION_MCAST}; 
char strSrcMulti[MAXHOSTNAME] = {DESTINATION_MCAST}; 
u_short nDestPort = DESTINATION_PORT;
u_short nSrcPort  = DESTINATION_PORT;
SOCKET hSock = INVALID_SOCKET;
struct sockaddr_in stDestAddr, stSrcAddr;
BOOL   bSocketOpen;
char achOutBuf[MCBUFSIZE];
char achInBuf [MCBUFSIZE];

/*-----------------------------------------------------------
 * Function: WinMain()
 *
 * Description: entry point
 */
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInst,
        LPSTR lpszCmdLine, int nShow)  {
  WNDCLASS  wc;
  MSG       msg;

  lpszCmdLine = lpszCmdLine;  /* avoid warning */ 
 
  hInst = hInstance;
  hwnd = NULL;
      
  if (!hPrevInst) {
    wc.style        = (unsigned int)NULL;
    wc.lpfnWndProc  = MainWinProc;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = 0;
    wc.hInstance    = hInst;
    wc.hIcon        = LoadIcon(hInst,MAKEINTRESOURCE(MULTITST));
    wc.hCursor      = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground= GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = MAKEINTRESOURCE(MULTITST);
    wc.lpszClassName= szAppName;                         

    if (!RegisterClass(&wc)) {
      MessageBox (NULL, "Couldn't register Window Class",
          NULL, MB_OK|MB_SYSTEMMODAL|MB_ICONHAND);
      return (0);
    }
  }
        
  InitWndData(hwnd);		/* for window dressing */

  hwnd = CreateWindow (szAppName,
    "Multitst",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
    CW_USEDEFAULT, CW_USEDEFAULT,
    ((rcDeskTop.right-rcDeskTop.left)/2),  /* Window about half screen */ 
    ((rcDeskTop.bottom-rcDeskTop.top)/2),
    NULL,NULL,hInst,NULL);  
 
  if (hwnd == NULL) {
    MessageBox (NULL, "Couldn't create main window",
      NULL, MB_OK|MB_SYSTEMMODAL|MB_ICONHAND);
    return (0);
  }

  ShowWindow (hwnd, nShow);
  UpdateWindow(hwnd);
    	
  while ( GetMessage(&msg, NULL, 0, 0) ) {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
  }
    
  if (hDispBuf)
    GlobalFree(hDispBuf);

  return (msg.wParam);
} /* end WndMain() */

/*-----------------------------------------------------------
 * Function: MainWndProc()
 *
 * Description: procedure for our main window.
 */
LRESULT CALLBACK MainWinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static short cxChar, cyCaps, cyChar, cxClient, cyClient, nMaxWidth,
               nVscrollPos, nVscrollMax, nHscrollPos, nHscrollMax;
  FARPROC lpfnProc;
  HDC hdc;
  TEXTMETRIC tm;
  int WSAErr, WSAEvent, nRet;
 
  switch (msg) {
    case WM_CREATE:
      /*------------- Register with WinSock DLL -----------*/
      WSAInitFailed = 
        WSAStartup(WSA_VERSION, &stWSAData); 
      if (WSAInitFailed != 0)  {
        WSAperror(WSAInitFailed, "WSAStartup()", hInst);
        PostMessage(hwnd, WM_CLOSE, 0, 0L);
      }
      /* Initialize destination address structure */
      stDestAddr.sin_family       = PF_INET;
      stDestAddr.sin_addr.s_addr  = inet_addr (strDestMulti);
      stDestAddr.sin_port         = htons (nDestPort);
      
      /* get window vitals */
      hdc = GetDC(hwnd);
      GetTextMetrics(hdc, &tm);
      nCharWidth  = tm.tmAveCharWidth;
      nCharHeight = tm.tmHeight + tm.tmExternalLeading;
      ReleaseDC(hwnd, hdc);
  
      /* center window */
      CenterWnd (hwnd, NULL, TRUE);
      
      break;
    
    case WM_COMMAND:
      switch(wParam) {
        case WSA_ASYNC:
          /* We received a WSAAsyncSelect() FD_ notification message 
           *  Parse the message to extract FD_ event value and error
           *  value (if there is one).
           */
          WSAEvent = WSAGETSELECTEVENT (lParam);
          WSAErr   = WSAGETSELECTERROR (lParam);
          if (WSAErr) {
            /* Error in asynch notification message: display to user */
            int i,j;
            for (i=0, j=WSAEvent; j; i++, j>>=1); /* convert bit to index */
            WSAperror(WSAErr, aszWSAEvent[i], hInst);
            /* fall-through to call reenabling function for this event */
          }
          switch (WSAEvent) {
            int tmp;
            case FD_READ:
              /* Receive the available data */
              tmp = sizeof(struct sockaddr);
              nRet = recvfrom (hSock, (char FAR *)achInBuf, MCBUFSIZE, 0,
                (struct sockaddr *) &stDestAddr, &tmp);
              if (nRet == SOCKET_ERROR) {
                WSAErrMsg ("recvfrom()");
                break;
              }
              /* Display the data received.*/
              MessageBox (hwnd, (LPSTR)achInBuf, 
		      "Multicast received!", MB_SYSTEMMODAL);
              break;
          } /* end switch(WSAEvent) */
          break;
        case IDM_NEWSOCKET:
          /* close current socket (if there is one) */
          if (hSock != INVALID_SOCKET) {        
            nRet = closesocket (hSock);
            if (nRet == SOCKET_ERROR) {
              WSAErrMsg("closesocket()");
            }
          }
          hSock = socket (PF_INET, SOCK_DGRAM, 0);
          if (hSock == INVALID_SOCKET) {
            WSAErrMsg ("socket()");
          } else {
            /*
             * Register for FD_READ events and post a WSA_ASYNC message
             */
            nRet = WSAAsyncSelect (hSock, hwnd, WSA_ASYNC, FD_READ);
            if (nRet == SOCKET_ERROR) {
              WSAErrMsg("WSAAsyncSelect(FD_READ)");
            } else {
               wsprintf ((LPSTR)achOutBuf, 
                 "Socket %d registered for FD_READ", hSock);
               MessageBox (hwnd, (LPSTR)achOutBuf, 
                 "Ready to Send", MB_OK | MB_ICONASTERISK);
            }
          }
          break;
        case IDM_BIND:
          lpfnProc = MakeProcInstance((FARPROC)BindDlgProc, hInst);
          DialogBoxParam (hInst, 
            "BindDlg", 
            hwnd, 
            (DLGPROC)lpfnProc,
            MAKELPARAM(hInst, hwnd));
          FreeProcInstance((FARPROC) lpfnProc);
          break;
        case IDM_SOCKOPTS:
          lpfnProc = MakeProcInstance((FARPROC)OptionDlgProc, hInst);
          DialogBoxParam (hInst, 
            "MulticstDlg", 
            hwnd, 
            (DLGPROC)lpfnProc,
            MAKELPARAM(hInst, hwnd));
          FreeProcInstance((FARPROC) lpfnProc);
          break;
        case IDM_SENDTO:
          lpfnProc = MakeProcInstance((FARPROC)SendDlgProc, hInst);
          DialogBoxParam (hInst, 
            "SendDlg", 
            hwnd, 
            (DLGPROC)lpfnProc,
            MAKELPARAM(hInst, hwnd));
          FreeProcInstance((FARPROC) lpfnProc);
          break;
        case IDM_PING:
          lpfnProc = MakeProcInstance((FARPROC)PingDlgProc, hInst);
          DialogBoxParam (hInst, 
            "PingDlg", 
            hwnd, 
            (DLGPROC)lpfnProc,
            MAKELPARAM(hInst, hwnd));
          FreeProcInstance((FARPROC) lpfnProc);
          break;
        case IDM_EXIT:
          PostMessage (hwnd, WM_CLOSE, 0, 0L);
          break;
        case IDM_ABOUT:
          DialogBox (hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, Dlg_About);
          break;
      } /* end switch(wParam) */
      break;
    case WM_DESTROY:
      if (!WSAInitFailed) {
        if (hSock != INVALID_SOCKET)
          closesocket (hSock);
        WSACleanup();
      }
      PostQuitMessage (0);
      break;
    case WM_PAINT:
      UpdateWnd(hwnd);
      break;
    default:
      return (DefWindowProc(hwnd, msg, wParam, lParam));
  } /* end switch(msg) */
  
  return (0L);
} /* end MainWndProc() */

/*-----------------------------------------------------------
 * Function: BindDlgProc()
 *
 * Description: procedure for the bind() dialog box
 */
BOOL CALLBACK BindDlgProc (
  HWND hDlg,
  UINT msg,
  UINT wParam,
  LPARAM lParam)
{
  static HANDLE hwnd, hInst;
  static u_long lMultiAddr;
  int nRet;
  
  switch (msg) {
    case WM_INITDIALOG:
            
      /* get parameters passed */
      if (lParam) {        
        hInst = (HANDLE)LOWORD(lParam);
        hwnd  = (HANDLE)HIWORD(lParam);
      }
                                              
      /* set display values */
      SetDlgItemText (hDlg, IDC_SRCADDR, strSrcMulti);
      SetDlgItemInt  (hDlg, IDC_SRCPORT, nSrcPort, FALSE);
      SetFocus (GetDlgItem (hDlg, IDC_BIND));

      /* center dialog box */
      CenterWnd (hDlg, hwnd, TRUE);
      
      return FALSE;
        
    case WM_COMMAND:
      switch (wParam) {

        case IDCANCEL:        
        case IDOK:
          EndDialog (hDlg, TRUE);
          return TRUE;
                
        case IDC_BIND:
          nSrcPort = (u_short) GetDlgItemInt (hDlg, IDC_SRCPORT, NULL, FALSE);
          GetDlgItemText (hDlg, IDC_SRCADDR, strSrcMulti, 40);
          lMultiAddr = inet_addr(strSrcMulti);
          if (lMultiAddr == INADDR_NONE) {
            WSAErrMsg("inet_addr()");
          } else {
            /* init source address structure to port & address requested */
            stSrcAddr.sin_family = PF_INET;
            stSrcAddr.sin_port   = htons(nSrcPort);
            stSrcAddr.sin_addr.s_addr = lMultiAddr;

            /* call bind */
            nRet = bind (hSock, 
              (struct sockaddr FAR *)&stSrcAddr, 
              sizeof(struct sockaddr));
            if (nRet == SOCKET_ERROR) {
              WSAErrMsg ("bind()");
            }
          }
          return TRUE;
        } /* end switch(wParam) */
      return TRUE;
    } /* end switch(msg) */
    return FALSE;
} /* end BindDlgProc() */

/*-----------------------------------------------------------
 * Function: SendDlgProc()
 *
 * Description: procedure for the send() dialog box
 */
BOOL CALLBACK SendDlgProc (
  HWND hDlg,
  UINT msg,
  UINT wParam,
  LPARAM lParam)
{
  static int net, nDataLen;
  static HANDLE hwnd, hInst;
  static u_long lMultiAddr;
  int nRet;
  
   switch (msg) {
      case WM_INITDIALOG:
            
      /* get parameters passed */
      if (lParam) {        
        hInst = (HANDLE)LOWORD(lParam);
        hwnd  = (HANDLE)HIWORD(lParam);
      }
                                              
      /* set display values */
      lstrcpy (achOutBuf, "Testing Multicast 1-2-3 (cha-cha-cha)");
      SetDlgItemText (hDlg, IDC_DESTADDR, strDestMulti);
      SetDlgItemText (hDlg, IDC_DATAOUT, achOutBuf);
      SetDlgItemInt  (hDlg, IDT_FPORT, nDestPort, FALSE);
    
      SetFocus (GetDlgItem (hDlg, IDC_SENDTO));

      /* center dialog box */
      CenterWnd (hDlg, hwnd, TRUE);
      
      return FALSE;
        
    case WM_COMMAND:
        switch (wParam) {

        case IDCANCEL:        
            case IDOK:
                EndDialog (hDlg, TRUE);
                return TRUE;
                
            case IDC_DATAOUT:
            case IDC_DESTADDR:
              break;
            case IDC_SENDTO:
              nDestPort = (u_short) GetDlgItemInt (hDlg, IDT_FPORT, NULL, FALSE);
              GetDlgItemText (hDlg, IDC_DATAOUT, achOutBuf, 1024);
              GetDlgItemText (hDlg, IDC_DESTADDR, strDestMulti, 40);
              lMultiAddr = inet_addr(strDestMulti);
              if (lMultiAddr == INADDR_NONE) {
                WSAErrMsg("inet_addr()");
              } else {
            /* init destination address structure */
            stDestAddr.sin_family = PF_INET;
            stDestAddr.sin_port   = htons (nDestPort);
            _fmemcpy ((char FAR *)&(stDestAddr.sin_addr), 
              (char FAR *)&lMultiAddr, 4);
                      /* sendto() */
            nRet = sendto (hSock, (char FAR *)achOutBuf, lstrlen(achOutBuf), 0,
                     (struct sockaddr FAR *) &stDestAddr, sizeof(struct sockaddr));
            if (nRet == SOCKET_ERROR) {
              WSAErrMsg ("sendto()");
            }
            return (FALSE);
              }
              break;
            case IDC_CONNECT:
              nDestPort = (u_short) GetDlgItemInt (hDlg, IDT_FPORT, NULL, FALSE);
              GetDlgItemText (hDlg, IDC_DESTADDR, strDestMulti, 40);
              lMultiAddr = inet_addr(strDestMulti);
              if (lMultiAddr == INADDR_NONE) {
                WSAErrMsg("inet_addr()");
              } else {
            /* init destination address structure */
            stDestAddr.sin_family = PF_INET;
            stDestAddr.sin_port   = htons (nDestPort);
            _fmemcpy ((char FAR *)&(stDestAddr.sin_addr), 
              (char FAR *)&lMultiAddr, 4);
                      /* connect */
            nRet = connect (hSock, 
                (struct sockaddr FAR *) &stDestAddr, 
                sizeof(struct sockaddr));
            if (nRet == SOCKET_ERROR) {
              WSAErrMsg ("connect()");
            }
            return (FALSE);
              }
            case IDC_SEND:
              GetDlgItemText (hDlg, IDC_DATAOUT, achOutBuf, MCBUFSIZE);
              nRet = send (hSock, (char FAR *)achOutBuf, lstrlen(achOutBuf), 0);
              if (nRet == SOCKET_ERROR) {
                WSAErrMsg ("send()");
              }
           return (FALSE);
        }
    return TRUE;
    }        
    return FALSE;
} /* end SendDlgProc() */

/*-----------------------------------------------------------
 * Function: OptionDlgProc()
 *
 * Description: procedure for the setsockopt()/getsockopt()
 *  dialog box
 */
BOOL CALLBACK OptionDlgProc (
  HWND hDlg,
  UINT msg,
  UINT wParam,
  LPARAM lParam)
{
  static int nRet, nOptName, nOptVal, nOptLen, nOptIDC, nLevel;
  static char FAR *lpOptVal;
  static HANDLE hwnd, hInst;
  static struct ip_mreq stIpReq;
  char achMultiIntr[MAXHOSTNAME];
  u_long lMultiAddr, lMultiIntr;
  struct in_addr stInAddr;
    
    switch (msg) {
        case WM_INITDIALOG:
            
            /* get parameters passed */
        if (lParam) {        
          hInst = (HANDLE)LOWORD(lParam);
          hwnd  = (HANDLE)HIWORD(lParam);
        }
                                              
      /* set display values */
      SetDlgItemInt  (hDlg, IDT_SOCKIN, hSock, FALSE);
      SetDlgItemText (hDlg, IDT_LEVELIN, "0");
      SetDlgItemText (hDlg, IDT_MULTIADDR, inet_ntoa (stDestAddr.sin_addr));
      SetDlgItemInt  (hDlg, IDT_MULTIINTR, IPPROTO_IP, FALSE);
      SetDlgItemInt  (hDlg, IDT_OPTNAME, IP_ADD_MEMBERSHIP, FALSE);    
      SetDlgItemInt  (hDlg, IDT_OPTVAL, 0, FALSE);
      SetDlgItemInt  (hDlg, IDT_OPTLEN, sizeof(struct ip_mreq), FALSE);
      CheckDlgButton (hDlg, IDC_BSD_OPTNAMES, bBSD_OptNames); 
      
      nOptName = IP_ADD_MEMBERSHIP;
           nOptLen  = sizeof (struct ip_mreq);
      nLevel   = IPPROTO_IP;
      lpOptVal = (char FAR *)&stIpReq;
    
      SetFocus (GetDlgItem (hDlg, IDT_MULTIADDR));
      CheckRadioButton(hDlg, IDR_ADD, IDR_OTHER, IDR_ADD); 

      /* center dialog box */
      CenterWnd (hDlg, hwnd, TRUE);
      
      break;
        
    case WM_COMMAND:
        switch (wParam) {

        case IDCANCEL:        
            case IDOK:
                EndDialog (hDlg, TRUE);
                break;
                
            case IDB_GETSOCKOPT:
            
              /* Get the parameter values, and call getsockopt(), 
               *  then display the results! */
              if (nOptName == USER_PARAMETER_VALUES) {
                nLevel   = GetDlgItemInt(hDlg, IDT_LEVELIN, NULL, FALSE);
                nOptName = GetDlgItemInt(hDlg, IDT_OPTNAME, NULL, FALSE);
                nOptVal  = GetDlgItemInt(hDlg, IDT_OPTVAL,  NULL, FALSE);
                  nOptLen  = GetDlgItemInt(hDlg, IDT_OPTLEN,  NULL, FALSE);
              }
              nRet = getsockopt(hSock, 
                nLevel, 
                (bBSD_OptNames ? nOptName : nOptName-DEERING_OFFSET),
                        lpOptVal,  /* pointer set to right location earlier */
                (int FAR *)&nOptLen);
              if (nRet == SOCKET_ERROR) {
                WSAErrMsg("getsockopt()");
               } else {
                /* Display the results */
            switch (nOptName) {
              case IP_ADD_MEMBERSHIP:    /* fail with get */
              case IP_DROP_MEMBERSHIP:
                break;
              case IP_MULTICAST_LOOP:
               case IP_MULTICAST_TTL:
              case USER_PARAMETER_VALUES:
                SetDlgItemInt  (hDlg, IDT_OPTVAL, *lpOptVal, FALSE);
                break;
              case IP_MULTICAST_IF:
                SetDlgItemText (hDlg, IDT_MULTIINTR, inet_ntoa(stInAddr));
                break;
            }
          }
              break;
              
            case IDB_SETSOCKOPT:
              /* Get the parameter values, and call getsockopt(), 
               *  then display the results! */
          switch (nOptName) {
            case IP_ADD_MEMBERSHIP:
            case IP_DROP_MEMBERSHIP:
                  GetDlgItemText (hDlg, IDT_MULTIADDR, 
                    strDestMulti, MAXHOSTNAME);
                  lMultiAddr = inet_addr(strDestMulti);
                  if (lMultiAddr == INADDR_NONE) {
                    WSAErrMsg("inet_addr()");                              
                  } else {
                    _fmemcpy ((char FAR *)&(stIpReq.imr_multiaddr),
                          (char FAR *)&lMultiAddr, 4);
                  }
                  GetDlgItemText (hDlg, IDT_MULTIINTR, 
                    achMultiIntr, MAXHOSTNAME);
                  lMultiIntr = inet_addr(achMultiIntr); 
                  /* we don't check inet_addr() error since INADDR_NONE is ok */
                  _fmemcpy ((char FAR *)&(stIpReq.imr_interface),
                        (char FAR *)&lMultiIntr, 4);
                  break;
                case IP_MULTICAST_TTL:
                case IP_MULTICAST_LOOP:
                  nOptVal = GetDlgItemInt(hDlg, IDT_OPTVAL, NULL, FALSE);
                  break;
                case IP_MULTICAST_IF:
                  GetDlgItemText (hDlg, IDT_MULTIINTR, 
                    achMultiIntr, MAXHOSTNAME);
                  lMultiIntr = inet_addr(achMultiIntr); 
                  /* we don't check inet_addr() error since INADDR_NONE is ok */
                  _fmemcpy ((char FAR *)&(stIpReq.imr_interface),
                        (char FAR *)&lMultiIntr, 4);
                  break;
                case USER_PARAMETER_VALUES:
                  /* Unknown option, so use user values for all input */
                  nLevel   = GetDlgItemInt(hDlg, IDT_LEVELIN, NULL, FALSE);
                  nOptName = GetDlgItemInt(hDlg, IDT_OPTNAME, NULL, FALSE);
                  nOptVal  = GetDlgItemInt(hDlg, IDT_OPTVAL,  NULL, FALSE);
                  nOptLen  = GetDlgItemInt(hDlg, IDT_OPTLEN,  NULL, FALSE);
                default:
                  break;
              }
              nRet = setsockopt(hSock,
                nLevel, 
                (bBSD_OptNames ? nOptName : nOptName-DEERING_OFFSET),
                lpOptVal,
                nOptLen);
              if (nRet == SOCKET_ERROR) {
                WSAErrMsg("setsockopt()");
              }
              /* Display the results (for better or worse) */
          SetDlgItemInt  (hDlg, IDT_OPTVAL, nOptVal, FALSE);
          break;
              
            case IDR_ADD:
              nLevel = IPPROTO_IP;
              nOptName = IP_ADD_MEMBERSHIP;
              nOptLen = sizeof (struct ip_mreq);
          lpOptVal = (char FAR *)&stIpReq;
          SetDlgItemInt  (hDlg, IDT_LEVELIN, IPPROTO_IP, FALSE);
          SetDlgItemInt  (hDlg, IDT_OPTNAME, IP_ADD_MEMBERSHIP, FALSE);    
          SetDlgItemInt  (hDlg, IDT_OPTVAL, 0, FALSE);
          SetDlgItemInt  (hDlg, IDT_OPTLEN, sizeof(struct ip_mreq), FALSE);
              break;
              
         case IDR_DROP:
           nLevel = IPPROTO_IP;
           nOptName = IP_DROP_MEMBERSHIP;
           nOptLen = sizeof (struct ip_mreq);
          lpOptVal = (char FAR *)&stIpReq;
          SetDlgItemInt  (hDlg, IDT_LEVELIN, IPPROTO_IP, FALSE);
          SetDlgItemInt  (hDlg, IDT_OPTNAME, IP_DROP_MEMBERSHIP, FALSE);    
          SetDlgItemInt  (hDlg, IDT_OPTVAL, 0, FALSE);
          SetDlgItemInt  (hDlg, IDT_OPTLEN, sizeof(struct ip_mreq), FALSE);
              break;
              
         case IDR_LOOP:
           nLevel = IPPROTO_IP;
           nOptName = IP_MULTICAST_LOOP;
           nOptLen = sizeof(int);
           nOptVal = IP_DEFAULT_MULTICAST_LOOP;
          lpOptVal = (char FAR *)&nOptVal;
          SetDlgItemInt  (hDlg, IDT_LEVELIN, IPPROTO_IP, FALSE);
          SetDlgItemInt  (hDlg, IDT_OPTNAME, IP_MULTICAST_LOOP, FALSE);    
          SetDlgItemInt  (hDlg, IDT_OPTVAL, IP_DEFAULT_MULTICAST_LOOP, FALSE);
          SetDlgItemInt  (hDlg, IDT_OPTLEN, sizeof(int), FALSE);
              break;
              
         case IDR_TTL:
           nLevel = IPPROTO_IP;
           nOptName = IP_MULTICAST_TTL;
           nOptLen = sizeof(int);
           nOptVal = IP_DEFAULT_MULTICAST_TTL;
          lpOptVal = (char FAR *)&nOptVal;
          SetDlgItemInt  (hDlg, IDT_LEVELIN, IPPROTO_IP, FALSE);
          SetDlgItemInt  (hDlg, IDT_OPTNAME, IP_MULTICAST_TTL, FALSE);    
          SetDlgItemInt  (hDlg, IDT_OPTVAL, IP_DEFAULT_MULTICAST_TTL, FALSE);
          SetDlgItemInt  (hDlg, IDT_OPTLEN, sizeof(int), FALSE);
              break;
              
         case IDC_IF:
           nLevel = IPPROTO_IP;
           nOptName = IP_MULTICAST_IF;
           nOptLen = sizeof(int);
           stInAddr.s_addr = INADDR_ANY;
          lpOptVal = (char FAR *)&stInAddr;
          SetDlgItemInt  (hDlg, IDT_LEVELIN, IPPROTO_IP, FALSE);
          SetDlgItemInt  (hDlg, IDT_OPTNAME, IP_MULTICAST_IF, FALSE);    
          SetDlgItemInt  (hDlg, IDT_OPTVAL, 0, FALSE);
          SetDlgItemInt  (hDlg, IDT_OPTLEN, sizeof(struct in_addr), FALSE);
              break;
              
            case IDR_OTHER:  /* at time of call, we'll use parameters */ 
              nOptName = USER_PARAMETER_VALUES;
              
            case IDC_BSD_OPTNAMES:  /* use Berkeley option names if set (or Deering if not) */
          bBSD_OptNames = !bBSD_OptNames;              
              
            case IDT_SOCKIN:
            case IDT_LEVELIN:
            case IDT_OPTVAL:
           break;
        } /* end switch(wParam) */
        
    return TRUE;
    } /* end switch(msg) */
            
    return FALSE;
} /* end OptionDlgProc() */

/*-----------s------------------------------------------------
 * Function: PingDlgProc()
 *
 * Description: procedure for the "raw" icmp and IP TTL dialog
 */
BOOL CALLBACK PingDlgProc (
  HWND hDlg,
  UINT msg,
  UINT wParam,
  LPARAM lParam)
{
  static int nDataLen = 64;  /* arbitrary data length (short to be safe) */
  static HANDLE hwnd, hInst;
  static int nIcmpId=1, nIcmpSeq=1;
  static int nIPTTL = 1;
  static SOCKET PingSocket = INVALID_SOCKET;
  struct sockaddr_in stDest, stSrc;
  char szHost[MAXHOSTNAME];
  u_long lRoundTripTime;
  int nRet, nOldId, nOldSeq, WSAErr, WSAEvent;
  static u_long lSendTime;
  
    switch (msg) {
    case WM_INITDIALOG:
            /* get parameters passed */
        if (lParam) {        
          hInst = (HANDLE)LOWORD(lParam);
          hwnd  = (HANDLE)HIWORD(lParam);
        }
      /* set display values */
      gethostname(szHost, MAXHOSTNAME);
      SetDlgItemText (hDlg, IDC_DEST, szHost);
      SetDlgItemInt  (hDlg, IDC_TTLVALUE, nIPTTL,  FALSE);
      SetDlgItemInt  (hDlg, IDC_DATALEN, nDataLen, FALSE);
      SetDlgItemInt  (hDlg, IDC_ICMPID,  nIcmpId,  FALSE);
      SetDlgItemInt  (hDlg, IDC_ICMPSEQ, nIcmpSeq, FALSE);
      SetDlgItemInt  (hDlg, IDC_SOCKET,PingSocket, FALSE);
      
      SetFocus (GetDlgItem (hDlg, IDC_SENDTO));

      /* center dialog box */
      CenterWnd (hDlg, hwnd, TRUE);
      
      break;
        
      case WSA_ASYNC:
         /* We received a WSAAsyncSelect() FD_ notification message 
          *  Parse the message to extract FD_ event value and error
          *  value (if there is one).
          */
         WSAEvent = WSAGETSELECTEVENT (lParam);
         WSAErr   = WSAGETSELECTERROR (lParam);
         if (WSAErr) {
           /* Error in asynch notification message: display to user */
            int i,j;
            for (i=0, j=WSAEvent; j; i++, j>>=1); /* convert bit to index */
            WSAperror(WSAErr, aszWSAEvent[i], hInst);
            /* fall-through to call reenabling function for this event */
         }
         switch (WSAEvent) {
          case FD_READ:
            lSendTime = icmp_recvfrom(PingSocket, 
              &nOldId, &nOldSeq, &stSrc);
            
            /* calculate round trip time, and display */ 
               lRoundTripTime = GetCurrentTime() - lSendTime;

          {
            char achErrBuf[MCBUFSIZE];
            wsprintf (achErrBuf, 
                "RoundTripTime: %ld ms from %s (ID: %d, Seq: %d)", 
                lRoundTripTime, inet_ntoa(stSrc.sin_addr), nOldId, nOldSeq);
                      
            MessageBox (hDlg, (LPSTR)achErrBuf, 
                "Ping received!", MB_OK | MB_ICONASTERISK);
		      
            UpdateDispBuf(hwnd, achErrBuf);
            InvalidateRect(hwnd, NULL, TRUE); 
            UpdateWindow(hwnd);
          }
          break;
        case FD_WRITE:
          break;
        default:
          break;
        }
        break;
    case WM_COMMAND:
        switch (wParam) {
        case IDCANCEL:        
            case IDQUIT:
              /* close our ping socket before we leave */
              closesocket(PingSocket);
              PingSocket = INVALID_SOCKET;
              
                EndDialog (hDlg, TRUE);
                break;
                
            case IDC_DATALEN:
            case IDC_DEST:
              break;
            case IDC_OPEN:
              /* Get an ICMP "raw" socket (first close the one we
               *  have open, if there is one. */
              if (PingSocket != INVALID_SOCKET) {
                icmp_close(PingSocket);
              }
              PingSocket = icmp_open();
              
              /* Register for asynchronous notification */
              nRet = WSAAsyncSelect (PingSocket, hDlg, WSA_ASYNC, FD_READ|FD_WRITE);
              if (nRet == SOCKET_ERROR) {
                WSAErrMsg("WSAAsyncSelect()");
              } else {
                SetDlgItemInt(hDlg, IDC_SOCKET, PingSocket, FALSE);
              }
              break;
              
            case IDOK:
            case IDC_SENDTO:
              if (IsDlgButtonChecked (hDlg, IDC_TTLENABLE)) {
                nIPTTL = GetDlgItemInt (hDlg, IDC_TTLVALUE, NULL, FALSE);
                set_ttl(PingSocket, nIPTTL++);
                SetDlgItemInt  (hDlg, IDC_TTLVALUE, nIPTTL, FALSE);
              }
              nIcmpId  = GetDlgItemInt (hDlg, IDC_ICMPID, NULL, FALSE);
              nIcmpSeq = GetDlgItemInt (hDlg, IDC_ICMPSEQ, NULL, FALSE);
              GetDlgItemText (hDlg, IDC_DEST, szHost, 64);
              stDest.sin_family = PF_INET;
              stDest.sin_addr.s_addr = GetAddr(szHost);
              stDest.sin_port = 0;
              icmp_sendto (PingSocket, 
                 hwnd, 
                 &stDest,
                   nIcmpId++,
                 nIcmpSeq++,
                 GetDlgItemInt (hDlg, IDC_DATALEN, NULL, FALSE));
              SetDlgItemInt (hDlg, IDC_ICMPID,  nIcmpId, FALSE);
              SetDlgItemInt (hDlg, IDC_ICMPSEQ, nIcmpSeq, FALSE); 
              break;
            default:
              return (FALSE);
        } /* end switch(wParam) */
        
    return (TRUE);
    } /* end switch(msg) */
    
    return FALSE;
} /* end PingDlgProc() */ 


/*-----------------------------------------------------------
 * Function: WSAErrMsg()
 *
 * Description: wrapper to simplify our generic error function
 */
void WSAErrMsg(LPSTR szFuncName) {
  WSAperror(WSAGetLastError(), szFuncName, hInst);
} /* end WSAErrMsg() */

/*
 * Function: InitWndData()
 *
 * Description: get info necessary to center windows, display text in the
 *  window and do scrolling.
 */
void InitWndData (HWND hwnd) {

  hDeskTop = GetDesktopWindow();

  GetWindowRect (hDeskTop, &rcDeskTop);
  
  nDeskTopWidth  = GetSystemMetrics (SM_CXSCREEN);
    nXCenter = nDeskTopWidth/2;
  nDeskTopHeight = GetSystemMetrics (SM_CYSCREEN);
    nYCenter = nDeskTopHeight/2;
     
  nMenuHeight    = GetSystemMetrics (SM_CYMENU);
  nCaptionHeight = GetSystemMetrics (SM_CYCAPTION);
  nScrollWidth   = GetSystemMetrics (SM_CXVSCROLL);
  nScrollHeight  = GetSystemMetrics (SM_CYHSCROLL);

  /* get display buffer */
  hDispBuf = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 
                         (DWORD) DISP_BUFSIZE);
  if (!hDispBuf) {
    MessageBox (hwnd, "Unable to allocate display buffer", "Error", 
               MB_OK | MB_ICONHAND);
  } else {
    lpDispBuf = GlobalLock(hDispBuf);
  }
  
} /* end InitWndData() */

/*
 * Function: UpdateDispBuf() 
 *
 * Description: Add a line to our display buffer and display it
 *  non rocket science kinda circular buffer in fixed line length
 *  two-demensional text buffer.  lots of corners cut, space wasted.
 */
void UpdateDispBuf(HWND hwnd, LPSTR szTextOut) {
	int i;
	LPSTR szOut;

    /* copy string to our display buffer */
    if (_fstrlen(szTextOut) > DISP_LINE_LEN)  /* truncate if too long */
    	*(szTextOut+DISP_LINE_LEN) = 0;
    szOut = lpDispBuf+(DISP_LINE_LEN*nLastLine);
    _fstrcpy (szOut, szTextOut);
    nLastLine++;
    if (nLastLine >= DISP_NUM_LINES) {
    	/* if we've reached the end of the buffer, we dump half the
    	 *  buffer by copying the bottom half over the top half.
    	 *  This ugly hack is due to laziness; we don't want to
    	 *  maintain circular buffer pointers. */
    	 nLastLine = DISP_NUM_LINES/2;
    	 i = nLastLine * DISP_LINE_LEN;
    	 szOut = lpDispBuf+i;
    	 _fmemcpy (lpDispBuf, szOut, i);
    	 _fmemset (szOut, 0, i);
    }
} /* end UpdateDispBuf() */  


/*
 * Function: UpdateWnd() 
 *
 * Description: WM_PAINT procedure
 */
void UpdateWnd(HWND hwnd) {
	HDC hdc;
	PAINTSTRUCT ps;
	int i, j;
	LPSTR szOut;
    
    /* point to first line in buffer to display */
    if (nLinesToDisplay < nLastLine) {
        /* point at last buffer line, then move it up to first display line */
    	szOut = (lpDispBuf+(DISP_LINE_LEN * nLastLine)) 
    	                  -(DISP_LINE_LEN * nLinesToDisplay);
    } else {
    	/* start display with first line in buffer */
    	szOut = lpDispBuf;
    }
                          
    /* display the text */
	hdc = BeginPaint(hwnd, &ps);

	for (i=0, j=nFirstLine; 
	     *szOut && i<nLinesToDisplay && j<DISP_NUM_LINES; 
	     i++, j++) {
	  TextOut (hdc, 10, (i*nCharHeight), szOut, _fstrlen(szOut));
	  szOut += DISP_LINE_LEN;
	} /* end for() */
	EndPaint(hwnd, &ps);
} /* end UpdateWnd() */
