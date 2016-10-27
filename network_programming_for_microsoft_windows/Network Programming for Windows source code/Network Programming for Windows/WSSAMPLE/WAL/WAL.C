/*---------------------------------------------------------------------
 *
 *  Program: WAL.EXE  WinSock Application Launcher
 *
 *  filename: wal.c
 *
 *  copyright by Bob Quinn, 1995
 *
 *  Description:
 *    WinSock test and performance analysis application that allows
 *    launching of tcp or udp, clients or servers, using blocking,
 *    non-blocking or asynchronous operation modes.  Socket options
 *    and many application I/O parameters are available to alter
 *    the application operation before or during execution.
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
#include <winsock.h> /* Windows Sockets */
#include <windowsx.h>	/* for WinNT */
#include <string.h> /* for _fmemcpy() & _fmemset() */

#include "..\winsockx.h"
#include "wal.h" 

/*------------ global variables ------------*/
char szWALClass   [] = "WinSock Application Launcher";

int  wChargenLen;				/* length of string to be output */
char achChargenBuf[2*BUF_SIZE];	/* we put CHARGEN_SEQ string resource here */
int  cbBufSize;

char achInBuf  [BUF_SIZE];  /* Input Buffer */
char achOutBuf [BUF_SIZE];  /* Output Buffer */

int recv_count;	/* number of calls to recv() on FD_READ */
                                                
/* socket name structures (for IP addresses & ports) */
struct sockaddr_in stLclSockName, stRmtSockName;

BOOL bShowDetail = FALSE;	/* toggle for detail statistics */

u_long lCallCount=0;	/* Total number I/O function calls */
WSAPP stWSAppData;		/* Application data */

/* Table of application ID's from which we launch based on user choice */
int anWSAppTable[2][2][3] = {	/* Role, Protocol, Mode */
	BL_DS_CL, NB_DS_CL, AS_DS_CL, 
	BL_DG_CL, NB_DG_CL, AS_DG_CL, 
	BL_DS_SV, NB_DS_SV, AS_DS_SV, 
	BL_DG_SV, NB_DG_SV, AS_DG_SV, 
};

/* strings describing the current socket state */
static LPSTR szSockState[] = {
	"none (no socket)", 
	"open", 
	"named (bound)",
	"listening",
	"accept pending",
	"connect pending",
	"connected",
	"close pending"
};

/* this table translates our I/O mode index into the WM_COMMAND I/O mode message */
int anIoCmd[] = {
	IDM_WRITE_READ,
	IDM_READ_WRITE,
	IDM_READ,
	IDM_WRITE
};

/* this table contains default settings and it's indexed by operation mode 
 *  (e.g. OPMODE_BL, OPMODE_NB, or OPMODE_AS) */
OPMODEOPTION astOpModeOption[] = {
	/* fields: nIoMode, nLength, nLoopLimit, nWinTimer, nBytesMax */
	{"Blocking",	IOMODE_WR,DIX_MSS,	DFLT_LOOP_MAX,200,0},	/* blocking mode */
	{"NonBlocking",	IOMODE_RW,DIX_MSS,	DFLT_LOOP_MAX,50, 0},	/* non-blocking mode */
	{"Asynchronous",IOMODE_R, DIX_MSS,	DFLT_LOOP_MAX,0,  0}	/* asynchronous mode */
};
/* BQ NOTE: I need to find out how to make GetProfileString look in the
 *  same directory where the executable resides */
char szIniFile[] = "WAL.INI";
char szAppOption[] = "AppOptions";

/* this table contains default settings, and it's indexed by I/O mode 
 * (e.g. read/write, write/read, read or write) and App role (client or server) */
IOMODEOPTION astIoModeOption[][2] = {
	/* Client Role:				Server Role: */
	{{ECHO_PORT, "echo"},		{CHARGEN_PORT, "chargen"}},	/* Write/Read mode */
	{{CHARGEN_PORT, "chargen"},	{ECHO_PORT, "echo"}},		/* Read/Write mode */
	{{CHARGEN_PORT, "chargen"},	{DISCARD_PORT, "discard"}},		/* Read-only mode */
	{{DISCARD_PORT, "discard"},	{CHARGEN_PORT, "chargen"}}	/* Write-only mode */
};

WSAPP stWSAppData;		/* Our Application's Datastructure */

HWND FAR PASCAL WALInit(HANDLE, HANDLE, int);
/*--------------------------------------------------------------------
 *  Function: WinMain()
 *
 *  Description: 
 *     initialize and start message loop
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
    HWND hwnd;
    
    hInst = hInstance;
    
    lpszCmdLine = lpszCmdLine; /* avoid warning */

    hwnd = WALInit(hInstance, hPrevInstance, nCmdShow);	/* initialize! */
    if (!hwnd) 	
        return 0;

	/*-------------Initialize Windows Sockets DLL------------*/
	nRet = WSAStartup(WS_VERSION_REQD, &stWSAData);
	/* WSAStartup() returns error value if failed (0 on success) */
	if (nRet != 0) {		
   		WSAperror(nRet, "WSAStartup()", hInst);
   	}
        	
    while (GetMessage (&msg, NULL, 0, 0)) {		/* main loop */
        TranslateMessage(&msg);
        DispatchMessage (&msg);
    }
    
	/*-------------say good-by to WinSock DLL----------------*/
	if (!nRet) {  /* only call if WSAStartup() succeeded */
	    nRet = WSACleanup();
	    if (nRet == SOCKET_ERROR) {
       		WSAperror(WSAGetLastError(), "WSACleanup()", hInst);
	    }
	}
    return msg.wParam;
} /* end WinMain() */


/*----------------------------------------------------------------------
 * Function: WALInit()
 *
 * Description: Register Window class, create and show main window.
 */
HWND FAR PASCAL WALInit(HANDLE hInst, HANDLE hPrevInst, int nCmdShow)
{
    WNDCLASS  wc;
    HWND 	hwnd;

	if (!hPrevInst) {
    	/* register WAL's window class */
    	wc.style         = CS_HREDRAW | CS_VREDRAW;
#pragma warning (disable:4050)
    	wc.lpfnWndProc   = WALWndProc;
    	wc.cbClsExtra    = 0;
    	wc.cbWndExtra    = 0;
    	wc.hInstance     = hInst;
    	wc.hIcon         = LoadIcon(hInst, IDWALICON);
    	wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
    	wc.hbrBackground = COLOR_APPWORKSPACE+1;
    	wc.lpszMenuName  = IDWALMENU;
    	wc.lpszClassName = szWALClass;
   		
   		if (!RegisterClass (&wc)) {
        	return (0);
        }
    }
    
    hwnd = CreateWindow(
    	szWALClass,
        szWALClass,
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

    if (!hwnd)
        return (0);
        
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return (hwnd);
    ;
} /* end WALInit() */
                    
/*----------------------------------------------------------------------
 * Function: do_stats()
 *
 * Description: Update on-screen statistics (we do this each time we
 *  get a timer message, or when user chooses UpdateStats command.
 */
void do_stats (HANDLE hwnd, HANDLE hInst, BOOL bUpdateDetail)
{   
	PAINTSTRUCT ps;
	HDC hdc;
	u_long lSeconds, lOutRate=0L, lInRate=0L;
	int i, wRet, nSockState, XPos, YPos;
	u_long lCallRate;

	/*
	 *   Display some stats.
	 */
	InvalidateRect(hwnd, NULL, TRUE);
	hdc = BeginPaint(hwnd, &ps);

    /* Calculate time elapsed (in seconds) iff we're connected */
    if (stWSAppData.nSockState == STATE_CONNECTED) {
		lSeconds = (GetTickCount() - stWSAppData.lStartTime); 
		if (lSeconds)  	/* avoid divide by zero */
			lSeconds /= 1000L;
		if (!lSeconds)  /* avoid divide by zero */
			lSeconds = 1L;
		stWSAppData.lSeconds = lSeconds;	/* time elapsed */
	} else {
		lSeconds = stWSAppData.lSeconds;	/* final time */
	}
						         
    XPos = X_START_POS;	/* start position for statistics output */
    YPos = Y_START_POS;						         
						         
	wsprintf(achOutBuf, "Bytes Out:     %lu   In: %lu", 
		stWSAppData.lBytesOut, stWSAppData.lBytesIn);
	TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
	YPos += STATTEXT_HGHT;
                       
	if (stWSAppData.lBytesOut)	/* avoid divide by zero */
		lOutRate = stWSAppData.lBytesOut/lSeconds;
	if (stWSAppData.lBytesIn)
		lInRate = stWSAppData.lBytesIn/lSeconds;
	wsprintf(achOutBuf, "Bytes/Sec Out: %lu   In: %lu",lOutRate, lInRate); 
	TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
	YPos += STATTEXT_HGHT;
			
	wsprintf(achOutBuf, "Time Up: %lu seconds", (lSeconds==1 ? 0 : lSeconds));
	TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
	YPos += STATTEXT_HGHT;

	wsprintf(achOutBuf, "Timer: %d,  Loops: %d (max: %d),  Bytes per IO: %d", 
		stWSAppData.nWinTimer, stWSAppData.nLoopLimit, 
		stWSAppData.nLoopMax, stWSAppData.nLength);
	TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
	YPos += STATTEXT_HGHT;

	nSockState = stWSAppData.nSockState;    
	for (i=0; nSockState; i++) {	 /* convert bit flag to index */
		nSockState = nSockState >> 1;
	}
	wsprintf(achOutBuf, "Socket: %d,  Socket State: %s",
		stWSAppData.nSock, szSockState[i]);
	TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
	YPos += STATTEXT_HGHT*2;	/* skip a line */
	
	/* If OOB enabled, show OOB data statistics */
	if (stWSAppData.nOptions & OPTION_OOBSEND) {
		wsprintf(achOutBuf, "OOB data bytes Out: %lu   In: %lu",
			stWSAppData.lOobBytesOut, stWSAppData.lOobBytesIn);
		TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
		YPos += STATTEXT_HGHT*2; /* skip a line */
	}
	
	/* Update Details (if requested and we have a socket) */
	if (bUpdateDetail && (stWSAppData.nSockState > STATE_NONE)) {
		int nLen = sizeof (struct sockaddr_in);
		
		_fmemset ((void FAR*)&stLclSockName, 0, nLen);
		_fmemset ((void FAR*)&stRmtSockName, 0, nLen);
		
		/* Get Local Socket Name (if we have a socket) */
		wRet = getsockname (stWSAppData.nSock,
			(struct sockaddr FAR *)&stLclSockName, 
			(int FAR*)&nLen);
		if (wRet == SOCKET_ERROR) {
			WSAperror((int)WSAGetLastError(), "getsockname()", hInst);
		} else if (stWSAppData.nSockState == STATE_CONNECTED) {
			/* Get Remote Socket Name (if we have an association) */
			wRet = getpeername (stWSAppData.nSock,
				(struct sockaddr FAR *)&stRmtSockName, 
				(int FAR *)&nLen);
			if (wRet == SOCKET_ERROR) {
				WSAperror(WSAGetLastError(),"getpeername()", hInst);
			}
		}
	} /* end if (bUpdateDetail) */
	
	/* If details were requested, show them (even if none available yet) */
	if (bShowDetail) {		
		char FAR *szIpAddr;
		
		switch (stWSAppData.nSockState) {
			case STATE_NONE:
				TextOut (hdc, XPos, YPos, 
					"No Socket Available", 19);
				break;
			case STATE_CONN_PENDING:
			case STATE_BOUND:
			case STATE_LISTENING:
			case STATE_ACCEPT_PENDING:
				TextOut (hdc, XPos, YPos, 
					"No Socket Association", 21);
				break;
			case STATE_CONNECTED:
			default:
				TextOut (hdc, XPos, YPos, 
					"getsockname() & getpeername() results:", 38);
		}
		YPos += STATTEXT_HGHT;

        szIpAddr = inet_ntoa (stLclSockName.sin_addr);
		wsprintf(achOutBuf, "Local Port: %d, IP Address: %s",
			htons (stLclSockName.sin_port),
			szIpAddr ? szIpAddr : "unknown");
		TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
		YPos += STATTEXT_HGHT;
        
        szIpAddr = inet_ntoa (stRmtSockName.sin_addr);
		wsprintf(achOutBuf, "Remote Port: %d, IP Address: %s",
			htons (stRmtSockName.sin_port),
			szIpAddr ? szIpAddr : "unknown");
		TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
		YPos += STATTEXT_HGHT*2;
		
		lCallRate = stWSAppData.lCallCount / lSeconds;
		wsprintf (achOutBuf, "I/O calls per second: %lu", lCallRate);
		TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
		YPos += STATTEXT_HGHT;
		
		wsprintf (achOutBuf, "back-to-back I/O calls %d (max: %d)",
			stWSAppData.nLoopsDn, stWSAppData.nLoopsDnMax);
		TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
		YPos += STATTEXT_HGHT;

		wsprintf (achOutBuf, "well spaced I/O calls %d (max: %d)",
			stWSAppData.nLoopsUp, stWSAppData.nLoopsUpMax);
		TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
		
#ifndef WIN32
		/* For debugging ...though hasn't revealed any problems yet 
		 *  (never seen more than 2 messages in task queue or 1 in
		 *  the system message queue. These functions are in DEBUG.C */
  		ShowWinTaskQ(hwnd);	/* output task queue info to debug */
		ShowSystemQ(hdc, XPos, YPos+STATTEXT_HGHT);
#endif
	}

	EndPaint(hwnd, &ps);
} /* end do_stats */

/*----------------------------------------------------------------------
 *
 * Function: get_timers()
 *
 * Description: We get one timer for statistic updates, and *another*
 *   timer if the user elects to launch a timer-driven application.
 *
 *  NOTE: We sometimes use *two* timers here.  Considering timers are a 
 *   limited resource, and especially since we could design it to use
 *   only one timer efficiently, this is not good.  I'm being lazy
 *   here, plain and simple, but then again the timer-driven app is
 *   not recommended anyway (it's just for experimentation).
 */
void get_timers (HANDLE hInst, HANDLE hwnd)
{
    hInst = hInst;	/* avoid warning */
    
	/* Set timer for statistics updates (every second) */
	if (!SetTimer (hwnd, STATS_TIMER_ID, STATS_INTERVAL, NULL)) {
		MessageBox (hwnd, 
			"Can't Get Windows Timer (for stats)", 
			"Error", MB_ICONEXCLAMATION | MB_OK);
	} else {
		KillTimer(hwnd, APP_TIMER_ID);
		 
		if (stWSAppData.nWinTimer) {
			/* Get timer if user wants timer-driven application */
			if (!SetTimer (hwnd, APP_TIMER_ID, 
				stWSAppData.nWinTimer, NULL)) {
				MessageBox (hwnd, 
					"Can't Get Windows Timer (for app)", "Error", 
					MB_ICONEXCLAMATION | MB_OK);
			}
		}
	}
	
	do_stats (hwnd, hInst, TRUE);	/* update stats display */
	
} /* end get_timers() */
                        
                        
/*----------------------------------------------------------------------
 * Function: SetNewTimer()
 *
 * Description: Set a new timeout period for our timer.
 */
void SetNewTimer (HANDLE hwnd, int nOldTimer, int nNewTimer)
{
	/* If we had a timer before, kill it (yea, this is unnecessary
	 *  since SetTimer() automatically kills the old timer, but
	 *  I *like* being paranoid). */
	if (nOldTimer)
		KillTimer (hwnd, APP_TIMER_ID);
		    				
	/* If we have a timer now, set it */
	if (nNewTimer) {
		if (!SetTimer (hwnd, APP_TIMER_ID, nNewTimer, NULL)) {
				MessageBox (hwnd, "Can't Get Windows Timer (for app)", 
					"Error", MB_ICONEXCLAMATION | MB_OK);
		}
	}
} /* end SetNewTimer() */

/*----------------------------------------------------------------------
 *
 * Function: LoopTimer()
 *
 * Description: Loop for a time-period, rather than waiting for reciept
 *  of a timer message (which is often unreliable on a busy system).
 */
void LoopTimer (BOOL IOStartFlag)
{
	static u_long lLastTicks;		/* Tick Count after last I/O */
	u_long lTicksSince;
	
	if (IOStartFlag)						/*** if ENTERING I/O loop */ 
	{
		lTicksSince = (GetTickCount() - lLastTicks);
		if (!lTicksSince) {  	/* if no time passed since last call */
			stWSAppData.nLoopsDn++;
			stWSAppData.nLoopsUp = 0;
			if (stWSAppData.nLoopsDn >= stWSAppData.nLoopsDnMax) {
				stWSAppData.nLoopsDn = 0;
				
				/* Decrease the number of loops per IO call, if possible */
				if (stWSAppData.nLoopLimit > 1)
					stWSAppData.nLoopLimit -= 1;
			}
		} else {
			stWSAppData.nLoopsUp++;
			stWSAppData.nLoopsDn = 0;
			if (stWSAppData.nLoopsUp >= stWSAppData.nLoopsUpMax) {
				stWSAppData.nLoopsUp = 0;
				
				/* Increment the number of loops per IO call, if it's
				 *  still less than the maximum value user set */
				if (stWSAppData.nLoopLimit < stWSAppData.nLoopMax)
					stWSAppData.nLoopLimit += 1;
			}
		}
		/* Increment the counter for the (calling) I/O function */
		stWSAppData.lCallCount++; 
	} 
	else									/*** if EXITING I/O loop ***/ 
	{
		/* Get Tick Count to see how soon we re-enter */
		lLastTicks = GetTickCount();
	}		
	return;
}	/* end LoopTimer() */

/*----------------------------------------------------------------------
 * Function: do_close()
 *
 * Description: Depending on the current socket state and the operation
 *  mode, we either initiate a close or complete a close.
 */
int do_close (HANDLE hInst, HANDLE hwnd, LPARAM bExitFlag)
{
	int wRet;
	int nOldState;
	
	wRet = IDOK;
	nOldState = stWSAppData.nSockState;
	
	if (nOldState == STATE_CONNECTED) {		/* connected? */
					
		/* it's premature to reset socket state here, 
		 *  but we do it to expedite the socket close */
		/*-----------------------------------------*/
		stWSAppData.nSockState = STATE_CLOSE_PENDING;
		/*-----------------------------------------*/
						
		/*---------------------------------------------------------------------*/
		/* NOTE: could add a dialog box here, to get closesocket timeout value */
		/*---------------------------------------------------------------------*/
		if (stWSAppData.nProtocol == PROT_DS) {	/* if it's a datastream socket */
		     wRet = MessageBox (				/*  ask if close gracefully */
				hwnd,                           /*  and set timeout = 0 if not */
	   			"Close Connection Gracefully?",
				"Close",
	    		MB_ICONQUESTION | MB_YESNOCANCEL); 

#if 0
            /*------NOTE: This method of blocking close DOES NOT WORK-------*/
            /* Check for and cancel pending blocking operation before we proceed! */
		   	if ((wRet != IDCANCEL) &&
		   		(WSAIsBlocking()))  {
		   		
		   		/* Cancel the blocking call */
		   		WSACancelBlockingCall();
		   		
		   		/* Now wait until the cancelled function returns (should
		   		 *  fail with WSAEINTR error), before we close the socket */
		   		do {
		   			OurBlockingHook();
		   		} while (WSAIsBlocking());
			} 
	    /*------------------ end of BAD example -----------------------*/
#endif	    	

	    	if (wRet == IDOK) {
				/* Connection abort requested, need to set
	 			 *  close timeout ("linger" value) to 0.
		 		 */
				struct linger stLinger;
				int WSAerr;
		
		   		stLinger.l_onoff = 1;   /* Linger On */
		   		stLinger.l_linger = 0;  /* 0 Seconds */
		   				
   				if (setsockopt(stWSAppData.nSock,
	           			SOL_SOCKET,
		   				SO_LINGER,
		   				(char FAR*) &stLinger,
		   				sizeof(struct linger))) {
		   			WSAerr = WSAGetLastError();
   					if (WSAerr == WSAEINPROGRESS) {
   						bl_close(hInst, hwnd);
   					} else {
        				WSAperror(WSAerr, "setsockopt()", hInst);
        			}
        		}
	    	}
	    } else {		/* if not Datastream, then it's Datagram */
	   		wRet = MessageBox (		/* ask if they're sure they want to close */
	    		hwnd,
	    		"Are you sure you want to close the socket?",
	    		"Close",
	    		MB_ICONQUESTION | MB_OKCANCEL);
	    }
	}  /* end if STATE_CONNECTED */
	    			 
	/*
	 * If we have a socket and operation is not cancelled, then close the socket
	 */
	if (stWSAppData.nSockState > STATE_NONE) {
	    if (wRet != IDCANCEL) {
			switch (stWSAppData.nOpMode) {
				case OPMODE_BL: 
					wRet = bl_close (hInst, hwnd);
					break;                          
				case OPMODE_NB:
				case OPMODE_AS:
					wRet = nb_close (hInst, hwnd);
					break;                          
			}
		} else {
			/* cancel close requested */
			stWSAppData.nSockState = nOldState;		/* reset state */
			if (nOldState == STATE_CONNECTED)		/* if it was connected, jumpstart */
				PostMessage(hwnd, WM_COMMAND, anIoCmd[stWSAppData.nIoMode], 0L);
			return TRUE;
		}	
	} else if (!bExitFlag) {
		MessageBox (hwnd, "No Sockets Open", "Can't Close", MB_OK | MB_ICONINFORMATION);
	}
	return FALSE;
} /* end do_close() */

/*--------------------------------------------------------------------
 * Function: WALWndProc()
 *
 * Description: Main window procedure
 */
LONG CALLBACK WALWndProc
	(HWND hwnd,
	 UINT msg,
	 WPARAM wParam,
	 LPARAM lParam)
{
    static HANDLE hInst;
    FARPROC lpfnWSAppDlgProc, lpfnWSOptionProc;
    int wRet, nFDError, nMsgQSize, i, j;
    HMENU hWalMenu;
    
    if (!stWSAppData.lBytesOut && !stWSAppData.lBytesIn)
    	stWSAppData.lStartTime = GetTickCount();	/* Start Time */
	                             
    switch (msg) {
   		case IDM_ASYNC:
   			/*----------------------------------------------
   			 * WSAAsyncSelect() event notification messages 
   			 *---------------------------------------------*/
   			nFDError = WSAGETSELECTERROR(lParam);		/* check async error */
   			if (nFDError) {
   				int nFDEvent = WSAGETSELECTEVENT(lParam);
   				
   				/* close the socket */
   				nb_close (hInst, hwnd);
   				
				/* if socket is active, notify user of error this asynch event message had */     
				if (!(stWSAppData.nSockState & (STATE_NONE | STATE_CLOSE_PENDING))) {
                    for (i=0, j=nFDEvent; j; i++, j>>=1); /* convert bit to index */
   					WSAperror(nFDError, aszWSAEvent[i], hInst);
   			    }
   			}
			switch (WSAGETSELECTEVENT(lParam)) {	/* check async event */
				case FD_OOB:
                     MessageBeep (MB_ICONASTERISK);
					 nb_oob_rcv (hInst, hwnd);
					 break;
				case FD_READ:
					/* we do multiple recv() calls per each FD_READ message we process,
					 *  so we keep count of the number and ignore that many FD_READ
					 *  messages.  The threshold is at 2 since 1 recv() is legitimate,
					 *  and the other may not have had data (if it failed with an
					 *  WSAEWOULDBLOCK error */
					if (recv_count > 2) {
						recv_count--;
						break;
					}

					/* rather than post the I/O message, we call the appropriate function to 
					 *  avoid the overhead involved with posting a message to proceed */
					 if (!(stWSAppData.nSockState  & (STATE_CONNECTED | STATE_BOUND))) {
					 	/* This message must have been left over 
					 	 *  in our message queue after a close */
					 	break;
					 }
					 switch (stWSAppData.nIoMode) {
					 	case IOMODE_R:
			 			case IOMODE_WR:
			 				/* do the read with more possible loops, to accomodate the
			 				 *  asynchronous nature and "bunching" of incoming packets */
			 				as_r (hInst, hwnd, (stWSAppData.nLoopLimit << 1));
			 				break;
			 			case IOMODE_RW:
			 				as_r_w (hInst, hwnd);
			 				break;
			 			case IOMODE_W:
			 				/* we shouldn't see the FD_READ event, so ask user what to do */
			 				wRet = MessageBox(hwnd, 
			 					"Data received, but application is write-only.  Read it?",
			 					"Unexpected Condition",
			 					MB_YESNO | MB_ICONQUESTION);
			 				if (wRet == IDYES)
			 					as_r (hInst, hwnd, DFLT_LOOP_MAX);
			 				break;
			 			}
					break;
				case FD_WRITE:
					/* Either we've just connected, so we're starting I/O for the first
					 *  time, or we've just got some outgoing buffers after a send()
					 *  failed with WSAEWOULDBLOCK so we should be able to send again. */
					 if (!(stWSAppData.nSockState  & (STATE_CONNECTED | STATE_BOUND))) {
					 	/* This message must have been left over 
					 	 *  in our message queue after a close */
					 	break;
					 }
					 switch (stWSAppData.nIoMode) {
					 	case IOMODE_R:
			 				/* nothing to do here since the application is read-only 
			 				 *  (its not sending any data).  We really shouldn't have 
			 				 *  registered for the FD_WRITE event, since our application
			 				 *  didn't need it.*/
			 				break;
			 			case IOMODE_WR:
			 			    /* write chargen data (printable ASCII character sequence 
			 			     *  we generate) */
			 				as_w_r (hInst, hwnd);
			 				break;
			 			case IOMODE_RW:
			 				/* we only have something to do for our "read then write" 
			 				 *  (server) application if we're recovering from a send()
			 				 *  that failed with a WSAEWOULDBLOCK error.  In that case
			 				 *  we should have a count of bytes left to write (where
			 				 *  we left off when our write failed). */
			 				if (stWSAppData.nOutLen) {
			 					as_r_w (hInst, hwnd);
			 				}
			 				break;
			 			case IOMODE_W:
			 				/* echo data that we received (the stWSAppData.nBytesToSend)*/
			 				as_w (hInst, hwnd);
			 				break;
			 		}
					break;
				case FD_ACCEPT:
					/* we have a connection request pending, accept it and start I/O */
					as_accept(hInst, hwnd);
					break;
				case FD_CONNECT:
					/* Set the socket state (if no error), and 
					 *  we'll start I/O with FD_WRITE */
					if (!nFDError) {
						/*-------------------------------------*/
						stWSAppData.nSockState = STATE_CONNECTED; 
						/*-------------------------------------*/
					} else {
						/* close the socket, if there's an error */
						do_close(hInst, hwnd, FALSE);
					}
					break;
				case FD_CLOSE:
					/* BQ NOTE: I just realized, I'd rather have my close functions call
					 *  shutdown(), recv() (or ioctlsocket(FIONREAD) if blocking socket), 
					 *  then call closesocket().
					 */
					if (stWSAppData.nSockState == STATE_CONNECTED)
						nb_close(hInst, hwnd);
			}
			break;

        case WM_COMMAND:
	    	switch (wParam) {
				case IDM_WRITE_READ:
					switch (stWSAppData.nOpMode) {
						case OPMODE_BL: 
							bl_w_r (hInst, hwnd);
							break;
						case OPMODE_NB:
							nb_w_r (hInst, hwnd);
							break;
						case OPMODE_AS:
							as_w_r (hInst, hwnd);
							break;
					}
					break;
				case IDM_READ_WRITE:
					switch (stWSAppData.nOpMode) {
						case OPMODE_BL: 
							bl_r_w (hInst, hwnd);
							break;
						case OPMODE_NB:
							nb_r_w (hInst, hwnd);
						case OPMODE_AS:
							break;	/* see IDM_ASYNC msg */
					}
					break;
				case IDM_WRITE:
					switch (stWSAppData.nOpMode) {
						case OPMODE_BL: 
							bl_w (hInst, hwnd);
							break;
						case OPMODE_NB:
							nb_w (hInst, hwnd);
						case OPMODE_AS:	
							as_w (hInst, hwnd);
							break;
				    }
					break;
				case IDM_READ:
					switch (stWSAppData.nOpMode) {
						case OPMODE_BL: 
							bl_r (hInst, hwnd);
							break;
						case OPMODE_NB:
							nb_r (hInst, hwnd);
							break;
						case OPMODE_AS:
							break;	/* see IDM_ASYNC msg */
					}
					break;
				case IDM_APPOPTIONS:
	    			/*-------------------------
	    			 * Start a new application 
	    			 *------------------------*/
					if (stWSAppData.nSockState > STATE_OPEN) {
						MessageBox (hwnd, "There's a socket open already", 
							"Can't start new application", MB_OK | MB_ICONASTERISK);
					} else {
					
					  lCallCount = 0;	/* init I/O call count to zero */

					  /* run dialog box to get user's WinSock application parameters */	    	
			    	  lpfnWSAppDlgProc = MakeProcInstance((FARPROC)WSAppOptDlgProc,hInst);
					  wRet = DialogBoxParam (hInst, 
							"AppOptionDlg", 
							hwnd, 
							lpfnWSAppDlgProc,
							MAKELPARAM(hInst, hwnd));
					  FreeProcInstance((FARPROC) lpfnWSAppDlgProc);
					
					  /* Reset stats */
					  SendMessage (hwnd, WM_COMMAND, IDM_RESETSTATS, 0L);
					
					  if (wRet) { /* If new app operation not cancelled */
								
						/* Set the Window Title to describe what we're running */
						wsprintf (achOutBuf, "WinSock %s %s %s (port %d)",
							(LPSTR)stWSAppData.szService,
							(LPSTR)((stWSAppData.nProtocol == PROT_DS) ? "stream" : "dgram"),
							(LPSTR)((stWSAppData.nRole == ROLE_CL) ? "client" : "server"),
							stWSAppData.nPortNumber);
						SetWindowText (hwnd, (LPSTR)achOutBuf);
				    
					  	/* User just selected the type of application and our dialog filled-in
					  	 *  our application structure.  Now look at what was selected */
					  	switch (anWSAppTable[stWSAppData.nRole]
										[stWSAppData.nProtocol]
										[stWSAppData.nOpMode]) {
										
                        	/*----- Datastream Clients ----------------------*/
   							case BL_DS_CL:	/* Blocking Datastream Client */
								bl_ds_cn(hInst, hwnd);
								break;
							case NB_DS_CL:	/* Non-blocking Datastream Client */
								nb_ds_cn(hInst, hwnd);
								break;
							case AS_DS_CL:	/* Asynch Datastream Client */
								as_ds_cn(hInst, hwnd);
								break;
						
                        	/*------ Datagram Clients ---------------------*/				
							case BL_DG_CL:  /* Blocking Datagram Client */
								bl_dg_cn(hInst, hwnd);
								break;
							case NB_DG_CL:  /* Non-blocking Datagram Client */
								nb_dg_cn(hInst, hwnd);
								break;
							case AS_DG_CL:  /* Asynch Datagram Client */
								as_dg_cn(hInst, hwnd);
								break;
						
                        	/*------ Datastream Servers -------------------*/
   							case BL_DS_SV:  /* Blocking Datastream Server */
								bl_ds_ls(hInst, hwnd);
								break;
							case NB_DS_SV:  /* Non-blocking Datastream Server */
								nb_ds_ls(hInst, hwnd);
								break;
							case AS_DS_SV:  /* Asynch Datastream Server */
								as_ds_ls(hInst, hwnd);
								break;
						
                        	/*------ Datagram Servers --------------------*/
							case BL_DG_SV:  /* Blocking Datagram Server */
								bl_dg_ls(hInst, hwnd);
								break;
							case NB_DG_SV:  /* Non-Blocking Datagram Server */
								nb_dg_ls(hInst, hwnd);
								break;
							case AS_DG_SV:  /* Asynch Datagram Server */
								as_dg_ls(hInst, hwnd);
								break;
						  }
					  	}
					}
					break;
				
				case IDM_OPENSOCK:
				    /*---------------------------------------------------------
				     * Dialog box to prompt for socket type, then get a socket 
				     *---------------------------------------------------------*/
        			/* If we already have a socket open, tell them to close it first */
        			if (stWSAppData.nSockState != STATE_NONE) {
        				MessageBox (hwnd, "There's a socket open already", "Can't open a socket", 
        					MB_OK | MB_ICONASTERISK);
        			} else {
			    		lpfnWSOptionProc = MakeProcInstance((FARPROC)WSSocketDlgProc, hInst);
						DialogBoxParam (hInst, 
							"SocketDlg", 
							hwnd, 
							lpfnWSOptionProc,
							MAKELPARAM(hInst, hwnd));
						FreeProcInstance((FARPROC) lpfnWSOptionProc);
					}
					break;
										               
				case IDM_ABORTSOCK:	      
				case IDM_CLOSESOCK:
	    			/*------------------------------------------------------------------
	    			 * Close the socket (if we have one and close not already requested)
	    			 *-----------------------------------------------------------------*/
	    			if ((stWSAppData.nSockState != STATE_NONE) &&
	    				(stWSAppData.nSockState != STATE_CLOSE_PENDING))
	    				return (do_close(hInst, hwnd, lParam));
	    			break;
	    			
				case IDM_IOOPTIONS:
					 /*----------------------------------
					  * Dialog box to change I/O options
					  *----------------------------------*/
					 lpfnWSOptionProc = MakeProcInstance((FARPROC)WSIOOptDlgProc, hInst);
					 DialogBoxParam (hInst,
					 		"IOOptionDlg",
					 		hwnd,
					 		lpfnWSOptionProc,
					 		MAKELPARAM(hInst, hwnd));
					 FreeProcInstance((FARPROC) lpfnWSOptionProc);
					 break;
					 
				case IDM_OOBOPTIONS:
					/*-----------------------------------------
					 * Dialog box to change application options
					 *----------------------------------------*/
			    	lpfnWSOptionProc = MakeProcInstance((FARPROC)WSOobOptDlgProc, hInst);
					DialogBoxParam (hInst, 
							"OobOptionDlg", 
							hwnd, 
							lpfnWSOptionProc,
							MAKELPARAM(hInst, hwnd));
					FreeProcInstance((FARPROC) lpfnWSOptionProc);

					break;

				case IDM_SOCKOPTIONS:
					/*---------------------------------
					 * Dialog box to get socket options
					 *---------------------------------*/
			    	lpfnWSOptionProc = MakeProcInstance((FARPROC)WSSockOptDlgProc, hInst);
					DialogBoxParam (hInst, 
							"SockOptionDlg", 
							hwnd, 
							lpfnWSOptionProc,
							MAKELPARAM(hInst, hwnd));
					FreeProcInstance((FARPROC) lpfnWSOptionProc);

					break;
				
	    		case IDM_RESETSTATS:
					/*------------------
					 * Reset statistics
					 *----------------*/
					stWSAppData.lBytesOut 	= 0;	/* reset send byte count */
					stWSAppData.lBytesIn	= 0;	/* reset receive byte count */
					stWSAppData.wOffset 	= 0;	/* start with first char in sequence */
					stWSAppData.lCallCount	= 0;	/* reset numberof I/O calls so far */
					stWSAppData.lSeconds	= 1;	/* reset time elapsed */
					stWSAppData.nLoopsUp	= 0;	/* reset loop adjustment counters */
					stWSAppData.nLoopsDn	= 0;
					stWSAppData.nLoopLimit  = stWSAppData.nLoopMax;	/* put loop at limit */
					stWSAppData.lStartTime 	= GetTickCount();		/* new start time */
					
					do_stats(hwnd, hInst, FALSE);

	    			break;
					
				case IDM_SHOWSTATS:
					/*---------------------------
					 * Update statistics display
					 *--------------------------*/
					do_stats(hwnd, hInst, FALSE);
					break;
					
				case IDM_DETAILSTATS:
					/*---------------------------------------
					 * Show Local & Remote Sockname w/ stats
					 *--------------------------------------*/
					bShowDetail = !bShowDetail;	/* toggle detail on/off switch */
					hWalMenu = GetMenu(hwnd); 	/*  check the menu to show setting */
					CheckMenuItem (hWalMenu, 
						IDM_DETAILSTATS,
						MF_BYCOMMAND | (bShowDetail ? MF_CHECKED : MF_UNCHECKED));
					do_stats(hwnd, hInst, TRUE);	/* update stats display */
					break;
					
				case IDM_NEWLOG:
				case IDM_CLOSELOG:
					MessageBox (hwnd, "Not implemented yet", "Error",
						MB_OK | MB_ICONASTERISK);
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
       		
		case WM_PAINT:
			do_stats (hwnd, hInst, FALSE);
			break;
                    
		case WM_TIMER:
			/*-------------------------------------------
			 * Process stats display and I/O timer events
			 *-------------------------------------------*/
			switch (wParam) {
				case (STATS_TIMER_ID):
					do_stats (hwnd, hInst, FALSE);
					break;
                    
				case (APP_TIMER_ID):
				    /*
				     *  Post ourselves message to do I/O
				     *   (if we're connected, and not already doing something)
				     */
        	        if ((stWSAppData.nSockState == STATE_CONNECTED) && !WSAIsBlocking())
						PostMessage(hwnd, WM_COMMAND, anIoCmd[stWSAppData.nIoMode], 0L);
					break;
					
			} /* end case WM_TIMER */
        	break;

	    case WM_QUERYENDSESSION:
        case WM_CLOSE:
			/*------------------
			 * Close the Socket
			 *-----------------*/
            /* Ask if user wants to close connection before we leave */
        	if (!SendMessage(hwnd, WM_COMMAND, IDM_CLOSESOCK, 1L)) {
 				/* Send WM_DESTROY & WM_NCDESTROY to main window */  
            	DestroyWindow(hwnd);
            }
            break;

        case WM_CREATE:
	    	/* save instance handle */
	    	hInst = ((LPCREATESTRUCT) lParam)->hInstance;
	    	
 			/* Default (96) is # ASCII chars displayed (to get barberpole on screen)
  			 *  we'll use the size the user wants, if they enter a new value */
  			cbBufSize = DFLT_BYTES_PER_IO;
  			                                                                               
  			/*------------load string into our work buffer---------- */
 			LoadString (hInst, CHARGEN_SEQ, achInBuf, BUF_SIZE);	/* get our char string */
  			i = (2*BUF_SIZE) / cbBufSize;	/* number of iterations we need to copy string */
   			for (j=0; i > 0; i--, j+=cbBufSize) {	/* fill chargen buf w/ repeated char series */
   				_fmemcpy((LPSTR)&(achChargenBuf[j]), (LPSTR)achInBuf, cbBufSize);
   			}
   			i = (2*BUF_SIZE) % cbBufSize;	/* to finish, fill remainder of buffer (if any) */
   			if (i) {
   				j = ((2*BUF_SIZE) / cbBufSize) * cbBufSize;
   				_fmemcpy ((LPSTR)&(achChargenBuf[j]), (LPSTR)achInBuf, i);
   			}

			/* assign application default option values (from WAL.INI, if available) */
			stWSAppData.nSockState 	= STATE_NONE;		/* socket is a non-entity yet */
			stWSAppData.nOutLen		= 0;
			stWSAppData.lSeconds	= 1L;
			stWSAppData.nProtocol	= GetPrivateProfileInt(
				/* Protocol: Datastream or Datagram */
				(LPSTR)szAppOption, "Protocol", PROT_DS, szIniFile);
			stWSAppData.nRole 		= GetPrivateProfileInt(
				/* Role: Client or Server */
				(LPSTR)szAppOption, "Role", ROLE_CL, szIniFile);
			stWSAppData.nOpMode 		= GetPrivateProfileInt(
				/* Operation mode: blocking, non-blocking, or asynch*/
				(LPSTR)szAppOption, "Mode", OPMODE_BL, szIniFile);
			stWSAppData.nIoMode 		= GetPrivateProfileInt(
				/* I/O Mode (per op mode): read/write, write/read, read-only or write-only */
				(LPSTR)astOpModeOption[stWSAppData.nOpMode].szSection, 
				"IoMode", 
				astOpModeOption[stWSAppData.nOpMode].nIoMode, 
				szIniFile);
			stWSAppData.nLength 	= GetPrivateProfileInt(
				/* bytes to read and/or write each i/o */
				(LPSTR)astOpModeOption[stWSAppData.nOpMode].szSection,
				"BytesPerIo", 
				astOpModeOption[stWSAppData.nOpMode].nLength, 
				szIniFile);
			stWSAppData.nLoopLimit 	= GetPrivateProfileInt(
				/* number of i/o iterations each loop */
				(LPSTR)astOpModeOption[stWSAppData.nOpMode].szSection,
				"IoPerLoop", 
				astOpModeOption[stWSAppData.nOpMode].nLoopLimit, 
				szIniFile);
			stWSAppData.nWinTimer 	= GetPrivateProfileInt(
				/* time between loops (0 if timer not used) */
				(LPSTR)astOpModeOption[stWSAppData.nOpMode].szSection,
				"LoopInterval", 
				astOpModeOption[stWSAppData.nOpMode].nWinTimer, 
				szIniFile);
			stWSAppData.nBytesMax 	= (u_short) GetPrivateProfileInt(
				/* maximum number of bytes to transfer */
				(LPSTR)astOpModeOption[stWSAppData.nOpMode].szSection,
				"BytesToXfer", 
				astOpModeOption[stWSAppData.nOpMode].nBytesMax, 
				szIniFile);
			stWSAppData.nPortNumber = (u_short) GetPrivateProfileInt(
				/* defualt port number */
				(LPSTR)astOpModeOption[stWSAppData.nOpMode].szSection,
				"PortNumber",
				astIoModeOption[stWSAppData.nOpMode][stWSAppData.nRole].nPortNumber, 
				szIniFile);
			GetPrivateProfileString(
				/* default service name */
				(LPSTR)astOpModeOption[stWSAppData.nOpMode].szSection,
				"ServiceName", 
				astIoModeOption[stWSAppData.nIoMode][stWSAppData.nOpMode].szService,
				(LPSTR)stWSAppData.szService, 
				MAX_NAME_SIZE, 
				szIniFile);
			GetPrivateProfileString(
				/* default (remote) hostname or IP address */
				(LPSTR)astOpModeOption[stWSAppData.nOpMode].szSection,
				"Host", 
				"",
				(LPSTR)stWSAppData.szHost, 
				MAX_NAME_SIZE, 
				szIniFile);
			/* Note: Some of these should be added to the .INI file, 
			 *  including the Send, Poll, InOobSound, and OutOobSound options */
			stWSAppData.lOobBytesOut= 0;
			stWSAppData.lOobBytesIn = 0;
			stWSAppData.nOobOutLen  = 1;
			stWSAppData.nOobInLen 	= stWSAppData.nLength;
			stWSAppData.nOobInterval= stWSAppData.nLength;
			
			stWSAppData.nLoopMax 	= stWSAppData.nLoopLimit;
			stWSAppData.nLoopsLeft  = stWSAppData.nLoopLimit;
			stWSAppData.nLoopsUpMax = DFLT_LOOP_UP;
			stWSAppData.nLoopsDnMax = DFLT_LOOP_DN;
 
			/* reset stats */
			SendMessage (hwnd, WM_COMMAND, IDM_RESETSTATS, 0L);

			/* increase our message queue to the max available 
			 *  NOTE: this is just a precaution, but doesn't hurt */
			for (nMsgQSize = 120;
			    ((!SetMessageQueue(nMsgQSize)) && (nMsgQSize > 0)); 
			     nMsgQSize--);
			     
            CenterWnd(hwnd, NULL, TRUE);
           break;
 
    case WM_DESTROY:
            /*---------------------------------
             * Close main window & application
             *-------------------------------*/
	        /* set offset to negative value to stop loop (only needed by BAD monolithic APPS) */
            stWSAppData.wOffset = -1;

            PostQuitMessage(0);
            break;
             
            default:
              return (DefWindowProc(hwnd, msg, wParam, lParam));   
    }

    return 0;
        
} /* end WALWndProc() */

/*---------------------------------------------------------------------
 * Function: WSAppOptDlgProc()
 *
 * Description:
 *   prompt user for winsock application parameters:
 *    - client or server
 *    - hostname or address (remote or local)
 *    - protocol (datagram or datastream)
 *    - opmode (blocking, non-blocking, asynch, asynch/nb)
 *
 *   allocate memory for our winsock applications private data, and
 *   initialize the appriate fields, including application id field 
 *   (with id number for window class).
 */                                        
BOOL CALLBACK WSAppOptDlgProc (
	HWND hDlg,
	UINT msg,
	UINT wParam,
	LPARAM lParam)
{
	static int nProtocol, nRole, nMode;
    static int nNewService = FALSE, nNewPort = 0;
    static HANDLE hInst = 0;
    static HANDLE hwnd = 0;
    struct servent FAR *lpServent;
    FARPROC lpfnWSOptionProc;
                                                 
    switch (msg) {
        case WM_INITDIALOG:
        
    		if (lParam) {
		    	hInst = LOWORD(lParam);
		    	hwnd  = HIWORD(lParam);
		    }
	
        	switch (stWSAppData.nProtocol) {
        		case PROT_DG:
        			nProtocol = IDC_DATAGRAM;
        			break;
        		case PROT_DS:
        			nProtocol = IDC_DATASTREAM;
        	}
        	CheckRadioButton (hDlg, IDC_DATASTREAM, IDC_DATAGRAM, nProtocol);
        	switch (stWSAppData.nRole) {
        		case ROLE_CL:
        			nRole = IDC_CLIENTAPP;
        			break;
        		case ROLE_SV:
        			nRole = IDC_SERVERAPP;
        	}
			CheckRadioButton (hDlg, IDC_CLIENTAPP, IDC_SERVERAPP, nRole);
			switch (stWSAppData.nOpMode) {
				case OPMODE_BL:
					nMode = IDC_BLOCKING;
					break;
				case OPMODE_NB:
					nMode = IDC_NONBLOCKING;
					break;
				case OPMODE_AS:
					nMode = IDC_ASYNCH;
					break;
			}
			CheckRadioButton (hDlg, IDC_BLOCKING, IDC_ASYNCH, nMode);
			SetDlgItemInt(hDlg, IDC_PORTNUMBER, stWSAppData.nPortNumber, FALSE);
			SetDlgItemText(hDlg, IDC_SERVICENAME, stWSAppData.szService);
			SetDlgItemText(hDlg, IDC_HOSTID, stWSAppData.szHost);
		
			SetFocus (GetDlgItem (hDlg, IDC_HOSTID));
            CenterWnd(hDlg, hwnd, TRUE);

	    	return FALSE;
	    
		case WM_COMMAND:
	    	switch (wParam) {
				case IDOPTIONS:
					/* Call the options dialog box to further qualify */
			    	lpfnWSOptionProc = MakeProcInstance((FARPROC)WSIOOptDlgProc, hInst);
					DialogBoxParam (hInst, 
							"IOOptionDlg", 
							hwnd, 
							lpfnWSOptionProc,
							MAKELPARAM(hInst, hwnd));
					FreeProcInstance((FARPROC) lpfnWSOptionProc);
					
					/* In options user can select Read/Write capabilities, which may cause
					 *  us to select different default services ...so refresh relevant items */
					SetDlgItemInt(hDlg, IDC_PORTNUMBER, stWSAppData.nPortNumber, FALSE);
					SetDlgItemText(hDlg, IDC_SERVICENAME, stWSAppData.szService);
					return TRUE;
					
				case IDCANCEL:
					EndDialog (hDlg, FALSE);
					return FALSE;
					
	        	case IDOK:
	        		/* get the new values (if there are any) */
	        		GetDlgItemText (hDlg, IDC_HOSTID, stWSAppData.szHost, MAX_NAME_SIZE);
	        		if (nNewService)
	        			GetDlgItemText (hDlg, IDC_SERVICENAME, stWSAppData.szService, MAX_NAME_SIZE);
	        		if (nNewPort)
	        			stWSAppData.nPortNumber = 
	        				(u_short)GetDlgItemInt (hDlg, IDC_PORTNUMBER, NULL, FALSE);
	        			             
		            EndDialog (hDlg, TRUE);
	    	        return TRUE;
	    	        
	    	    case IDC_HOSTID:	/* BQ NOTE: might want to resolve it.  
	    	    						If so, might need an IP Addr field */
	    	    	return TRUE;
	    	    	
	    	    case IDC_SERVICENAME:	/* new service name, let's resolve it! */
	    	    	switch (HIWORD (lParam)) {
	    	    		static BOOL nChanged = FALSE;
	    	    		
	    	    		case EN_UPDATE:
	    	    			nChanged = TRUE;
	    	    			return FALSE;
	    	    			
	    	    		case EN_KILLFOCUS:
	    	    			if (nChanged) {
	    			    	  GetDlgItemText (hDlg, IDC_SERVICENAME, achInBuf, MAX_NAME_SIZE);
			    	    	  lpServent = getservbyname (achInBuf,	/* get port number from service name */
	    	    					      (LPSTR) (stWSAppData.nProtocol == PROT_DS ? "tcp" : "udp"));
	    			    	  if (lpServent) {	/* points to servent struct if successful (else NULL) */
								nNewPort = ntohs(lpServent->s_port);
								SetDlgItemInt(hDlg, IDC_PORTNUMBER, nNewPort, FALSE);
	    	    				nNewService = TRUE;
	    	    			  } else {
	    			    		SetDlgItemText(hDlg, IDC_PORTNUMBER, "<unknown>");
			    	    		nNewPort = 0;
			    	    		nNewService = FALSE;
	    	    			  }
	    	    			}
	    	    		default:
	    	    			return FALSE;
		    	    }	
	    	    	
	    	    case IDC_PORTNUMBER:	/* new port number, let's resolve it! */
	    	    	switch (HIWORD (lParam)) {
	    	    		static BOOL nChanged = FALSE;
	    	    		
	    	    		case EN_UPDATE:
	    	    			nChanged = TRUE;
	    	    			return FALSE;
	    	    			
	    	    		case EN_KILLFOCUS:
	    	    			if (nChanged) {
			        		  nNewPort = GetDlgItemInt (hDlg, IDC_PORTNUMBER, NULL, FALSE);
	    		    		  lpServent = getservbyport (htons((u_short)nNewPort),	/* get service name from port # */ 
	        							  (LPSTR) (stWSAppData.nProtocol == PROT_DS ? "tcp" : "udp"));
			        		  if (lpServent) {	/* points to servent struct if successful (else NULL) */
	    		    			SetDlgItemText (hDlg, IDC_SERVICENAME, lpServent->s_name);
	        					nNewService = TRUE;
			        		  } else {
	    		    			SetDlgItemText (hDlg, IDC_SERVICENAME, "<unknown>");
	        					nNewService = FALSE;
	    	    			  }
	    	    			}
	    	    		default:
	    	    			return FALSE;
		    	    }	
	    	        
	    	    case IDC_DATASTREAM:
	    	   		stWSAppData.nProtocol = PROT_DS;
					return TRUE;

	    	    case IDC_DATAGRAM:
					stWSAppData.nProtocol = PROT_DG;
					return TRUE;
					
	    	    case IDC_CLIENTAPP:
	    	    	stWSAppData.nRole = ROLE_CL;
	    	    	if (stWSAppData.nIoMode == IOMODE_RW)	/* fix I/O order, if necessary */
	    	    		stWSAppData.nIoMode = IOMODE_WR;
					return TRUE;
					
	    	    case IDC_SERVERAPP:
					stWSAppData.nRole = ROLE_SV;
					if (stWSAppData.nIoMode == IOMODE_WR)	/* fix I/O order, if necessary */
						stWSAppData.nIoMode = IOMODE_RW;
					return TRUE;
					
	    	    case IDC_BLOCKING:
					stWSAppData.nOpMode = OPMODE_BL;
					return TRUE;

	    	    case IDC_NONBLOCKING:
					stWSAppData.nOpMode = OPMODE_NB;
					return TRUE;

	    	    case IDC_ASYNCH:
					stWSAppData.nOpMode = OPMODE_AS;
					return TRUE;
	    	}
 		break;
    }        
    return FALSE;
    
} /* end WSAppOptDlgProc() */
	    
/*---------------------------------------------------------------------
 * Function: WSIOOptDlgProc()
 *
 * Description: I/O option dialog procedure to:
 *   - select read and/or write
 *   - turn on or off the I/O sound
 *	 - set read/write amounts
 *   - set I/O loop parameters
 */                                        
BOOL CALLBACK WSIOOptDlgProc (
	HWND hDlg,
	UINT msg,
	UINT wParam,
	LPARAM lParam)
{
	static int nReadFlag, nWriteFlag, nWinTimer;
	static HANDLE hwnd, hInst;
	static BOOL bSound;
	FARPROC lpfnWSIOAdvProc;
	
   switch (msg) {
        case WM_INITDIALOG:
            
            /* get parameters passed */
    		if (lParam) {				
		    	hInst = LOWORD(lParam);
		    	hwnd  = HIWORD(lParam);
		    }
                                              
            /* init local work variables from actual values */                                           
			nReadFlag  = ((stWSAppData.nIoMode==IOMODE_R)  || 
						  (stWSAppData.nIoMode==IOMODE_WR) ||
						  (stWSAppData.nIoMode==IOMODE_RW));
			nWriteFlag = ((stWSAppData.nIoMode==IOMODE_W) || 
			              (stWSAppData.nIoMode==IOMODE_WR) ||
			              (stWSAppData.nIoMode==IOMODE_RW));
			bSound = (stWSAppData.nOptions & OPTION_SOUND);

			/* set display values */
 			CheckDlgButton (hDlg, IDC_READ, nReadFlag);
			CheckDlgButton (hDlg, IDC_WRITE, nWriteFlag);
			CheckDlgButton (hDlg, IDC_SOUND, bSound);
			SetDlgItemInt (hDlg, IDC_BYTES_PER_IO, stWSAppData.nLength, FALSE);
			SetDlgItemInt (hDlg, IDC_IO_PER_LOOP, stWSAppData.nLoopLimit, FALSE);
			SetDlgItemInt (hDlg, IDC_MAX_IO_PER_LOOP, stWSAppData.nLoopMax, FALSE);
			SetDlgItemInt (hDlg, IDC_LOOP_INTERVAL, stWSAppData.nWinTimer, FALSE);
			SetDlgItemInt (hDlg, IDC_MAXBYTES, stWSAppData.nBytesMax, FALSE);
		
			SetFocus (GetDlgItem (hDlg, IDOK));
            CenterWnd(hDlg, hwnd, TRUE);

	    	return FALSE;
		case WM_COMMAND:
	    	switch (wParam) {
	    	
	    		case IDCANCEL:
	    		
	    			/* "Cancel" button pressed, so just leave (ignore any new values) */
		            EndDialog (hDlg, FALSE);
	    	        return TRUE;
	    	        
	        	case IDOK:
	        		
	        		/* "OK" button pressed, so get the new values (if there are any) */
	        		if (nReadFlag && nWriteFlag) {
	        			if (stWSAppData.nRole == ROLE_CL) {
	        				/* our client sends, then receives */
	        				stWSAppData.nIoMode = IOMODE_WR;
	        			} else {
	        				/* our server receives, then sends */
	        				stWSAppData.nIoMode = IOMODE_RW;
	        			}
	        			/* default service for client or server if both reading & writing */
	        			stWSAppData.nPortNumber = ECHO_PORT;
	        			_fmemcpy (stWSAppData.szService, "echo", 5);	
	        		} else if (nReadFlag) {
	        			stWSAppData.nIoMode = IOMODE_R;
	        			if (stWSAppData.nRole == ROLE_CL) {
	        				/* default service for client to connect to when read-only */
	        				stWSAppData.nPortNumber = CHARGEN_PORT;
	        				_fmemcpy (stWSAppData.szService, "chargen", 8);	
	        			} else {
	        				/* default service for server to provide when read-only */
	        				stWSAppData.nPortNumber = DISCARD_PORT;
	        				_fmemcpy (stWSAppData.szService, "discard", 8);	
	        			}
	        		} else if (nWriteFlag) {
	        			stWSAppData.nIoMode = IOMODE_W;
	        			if (stWSAppData.nRole == ROLE_CL) {
	        				/* default service for client to connect to when write-only*/
	        				stWSAppData.nPortNumber = DISCARD_PORT;
	        				_fmemcpy (stWSAppData.szService, "discard", 8);	
	        			} else {
	        				/* default service for server to provide when write-only */
	        				stWSAppData.nPortNumber = CHARGEN_PORT;
	        				_fmemcpy (stWSAppData.szService, "chargen", 8);	
	        			}
	        		}
	        			
	        		if (!stWSAppData.nIoMode)    /* if none selected, do both! */
	        			stWSAppData.nIoMode = IOMODE_WR;
	        			                
	        		stWSAppData.nOptions |= OPTION_SOUND;                         
	        		stWSAppData.nOptions &= (bSound ? OPTION_MASK : ~OPTION_SOUND);
	        			
	        		stWSAppData.nLength = 
	        			GetDlgItemInt (hDlg, IDC_BYTES_PER_IO, NULL, FALSE);
	        		stWSAppData.nLoopLimit = 
	        			GetDlgItemInt (hDlg, IDC_IO_PER_LOOP, NULL, FALSE);
	        		stWSAppData.nLoopsLeft = stWSAppData.nLoopLimit;
	        		stWSAppData.nLoopMax = 
	        			GetDlgItemInt (hDlg, IDC_MAX_IO_PER_LOOP, NULL, FALSE);
	        		stWSAppData.nBytesMax = 
	        			(u_short) GetDlgItemInt (hDlg, IDC_MAXBYTES, NULL, FALSE);

					/* Get timer value; maybe it's new!? */
	        		nWinTimer = GetDlgItemInt (hDlg, IDC_LOOP_INTERVAL, NULL, FALSE);
	        		
	        		/* If timer value changed, change things to use new value */
	        		if (nWinTimer != stWSAppData.nWinTimer) {
	        			
	        			if (stWSAppData.nSockState == STATE_CONNECTED) {
	        				SetNewTimer(hwnd, stWSAppData.nWinTimer, nWinTimer);
						}
						stWSAppData.nWinTimer = nWinTimer;	/* save the new timer value */
					}
	        		
		            EndDialog (hDlg, TRUE);
	    	        return TRUE;
	    	        
	    	    case IDC_READ:
	    	    	nReadFlag = !nReadFlag;
	    	    	return TRUE;
	    	    	
	    	    case IDC_WRITE:
	    	    	nWriteFlag = !nWriteFlag;
	    	    	return TRUE;
	    	    	
			 	case IDC_SOUND:
			 		bSound = !bSound;
	    	    	return TRUE;
	    	    	
	    	    case IDB_IOADVANCED:
					/* Call the advanced options dialog box to further qualify */
			    	lpfnWSIOAdvProc = 
			    		MakeProcInstance((FARPROC)WSIOAdvDlgProc, hInst);
					DialogBox (hInst, 
						"IOAdvancedDlg", 
						hwnd, 
						lpfnWSIOAdvProc);
					FreeProcInstance((FARPROC) lpfnWSIOAdvProc);
	    	    	
	    			return TRUE;    	
	    	    	
	    	    case IDC_BYTES_PER_IO:
	    	    case IDC_IO_PER_LOOP:
	    	    case IDC_MAX_IO_PER_LOOP:
	    	    case IDC_LOOP_INTERVAL:
	    	    case IDC_MAXBYTES:
			 		return TRUE;
	    	}
 		break;
    }        
    return FALSE;
    
} /* end WSIOOptDlgProc() */
	    
/*---------------------------------------------------------------------
 * Function: WSIOAdvDlgProc()
 *
 * Description: Advanced dialog procedure, used to adjust the obscure
 *   automatic loop adjustment parameters (used for Asynch only).
 */
BOOL CALLBACK WSIOAdvDlgProc (
	HWND hDlg,
	UINT msg,
	UINT wParam,
	LONG lParam)
{
	lParam = lParam;	/* avoid warning */
    wParam = wParam;
    
    switch (msg) {
        case WM_INITDIALOG:
			SetDlgItemInt (hDlg, IDC_LOOPUP, stWSAppData.nLoopsUpMax, FALSE);
			SetDlgItemInt (hDlg, IDC_LOOPDN, stWSAppData.nLoopsDnMax, FALSE);
	    	return FALSE;
	    
		case WM_COMMAND:
	    	switch (wParam) {
	    		case IDC_LOOPUP:
	    		case IDC_LOOPDN:
	    			return TRUE;
	    			
	        	case IDOK:
	        		stWSAppData.nLoopsUpMax = GetDlgItemInt (hDlg, IDC_LOOPUP, NULL, FALSE);
	        		stWSAppData.nLoopsDnMax = GetDlgItemInt (hDlg, IDC_LOOPDN, NULL, FALSE);

					/* fall through */	        		
	    		case IDCANCEL:
	            	EndDialog (hDlg, 0);
	            	return TRUE;
	    	}
		break;
    }
    return FALSE;
} /* end WSIOAdvDlgProc() */

/*---------------------------------------------------------------------
 * Function: WSSockOptDlgProc()
 *
 * Description: Dialog procedure to set and/or get any socket option
 *   using setsockopt() and/or getsockopt().
 */                                        
BOOL CALLBACK WSSockOptDlgProc (
	HWND hDlg,
	UINT msg,
	UINT wParam,
	LPARAM lParam)
{
	static int wRet, nOptName, nOptVal, nOptLen, nOptIDC, nLevel, WSAerr;
	static HANDLE hwnd, hInst;
	static struct linger stLinger;
	
   switch (msg) {
        case WM_INITDIALOG:
            
            /* get parameters passed */
    		if (lParam) {				
		    	hInst = LOWORD(lParam);
		    	hwnd  = HIWORD(lParam);
		    }
                                              
			/* set display values */
 			CheckDlgButton (hDlg, IDC_ACCEPTCON, TRUE);
			SetDlgItemInt  (hDlg, IDC_SOCKIN, stWSAppData.nSock, FALSE);
			SetDlgItemText (hDlg, IDC_LEVELIN, "SOL_SOCKET");
			SetDlgItemInt  (hDlg, IDC_OPTVAL, 0, FALSE);
			SetDlgItemInt  (hDlg, IDC_OPTLEN, 2, FALSE);
			SetDlgItemInt  (hDlg, IDC_LINGERFLAG, 0, FALSE);
			SetDlgItemInt  (hDlg, IDC_LINGERSECS, 0, FALSE);
			
			nOptName = SO_ACCEPTCONN;
		
			SetFocus (GetDlgItem (hDlg, IDB_GETSOCKOPT));
            CenterWnd(hDlg, hwnd, TRUE);

	    	return FALSE;
	    	
		case WM_COMMAND:
	    	switch (wParam) {

				case IDCANCEL:	    	
	        	case IDOK:
		            EndDialog (hDlg, TRUE);
	    	        return TRUE;
	    	        
	    	    case IDB_GETSOCKOPT:
	    	    
	    	    	/* Get the parameter values, and call getsockopt(), 
	    	    	 *  then display the results! */
					nLevel = ((nOptName == TCP_NODELAY) ? IPPROTO_TCP : SOL_SOCKET);
	    	    	nOptLen = sizeof(int);
	    	    	wRet = getsockopt(stWSAppData.nSock, 
	    	    		nLevel, 
	    	    		nOptName, 
	    	    		((nOptName == SO_LINGER) ? 
	    	    			(char FAR *)&stLinger : (char FAR *)&nOptVal), 
	    	    		(int FAR *)&nOptLen);
	    	    	if (wRet == SOCKET_ERROR) {
	    	    		WSAerr = WSAGetLastError();
        				WSAperror(WSAerr, "getsockopt()", hInst);
	    	    	} else if (nOptName == SO_LINGER) {
	    	    		SetDlgItemInt (hDlg, IDC_LINGERFLAG,
	    	    			stLinger.l_onoff, FALSE);
	    	    		SetDlgItemInt (hDlg, IDC_LINGERSECS,
	    	    			stLinger.l_linger, FALSE);
	    	    	} else {
	    	    		/* Display the results */
						SetDlgItemInt  (hDlg, IDC_OPTVAL, nOptVal, FALSE);
					}
					
	    	    	return TRUE;
	    	    	
	    	    case IDB_SETSOCKOPT:
	    	    
	    	    	/* Get the parameter values, and call getsockopt(), 
	    	    	 *  then display the results! */
	    	    	if (nOptName == SO_LINGER) {
	    	    		stLinger.l_onoff = 
	    	    			(u_short) GetDlgItemInt (hDlg, IDC_LINGERFLAG, NULL, FALSE);
	    	    		stLinger.l_linger = 
	    	    			(u_short) GetDlgItemInt (hDlg, IDC_LINGERSECS, NULL, FALSE);
	    	    	} else {
	    	    		nOptVal = 
	    	    			(int) GetDlgItemInt (hDlg, IDC_OPTVAL, NULL, FALSE);
	    	    	}
					nLevel = ((nOptName == TCP_NODELAY) ? IPPROTO_TCP : SOL_SOCKET);
	    	    	nOptLen = sizeof(int);
	    	    	wRet = setsockopt(stWSAppData.nSock,
	    	    		nLevel, 
	    	    		nOptName, 
	    	    		((nOptName == SO_LINGER) ? 
	    	    			(char FAR *)&stLinger : (char FAR *)&nOptVal), 
	    	    		nOptLen);
	    	    	if (wRet == SOCKET_ERROR) {
	    	    		WSAerr = WSAGetLastError();
        				WSAperror(WSAerr, "setsockopt()", hInst);
	    	    	} else if (nOptName == SO_LINGER) {
	    	    		SetDlgItemInt (hDlg, IDC_LINGERFLAG,
	    	    			stLinger.l_onoff, FALSE);
	    	    		SetDlgItemInt (hDlg, IDC_LINGERSECS,
	    	    			stLinger.l_linger, FALSE);
	    	    	} else {
	    	    		/* Display the results */
						SetDlgItemInt  (hDlg, IDC_OPTVAL, nOptVal, FALSE);
					}
					
	    	    	return TRUE;
	    	    	
	    	    case IDC_ACCEPTCONN:
	    	    	nOptName = SO_ACCEPTCONN;
	    	    	break;
	    	    	
			 	case IDC_BROADCAST:
			 		nOptName = SO_BROADCAST;
	    	    	break;
	    	    	
			 	case IDC_DEBUG:
			 		nOptName = SO_DEBUG;
	    	    	break;
	    	    	
			 	case IDC_DONTLINGER:
			 		nOptName = SO_DONTLINGER;
	    	    	break;
	    	    	
			 	case IDC_DONTROUTE:
			 		nOptName = SO_DONTROUTE;
	    	    	break;
	    	    	
			 	case IDC_ERROR:
			 		nOptName = SO_ERROR;
	    	    	break;
	    	    	
			 	case IDC_KEEPALIVE:
			 		nOptName = SO_KEEPALIVE;
	    	    	break;
	    	    	
			 	case IDC_LINGER:
			 		nOptName = SO_LINGER;
	    	    	break;
	    	    	
			 	case IDC_OOBINLINE:
			 		nOptName = SO_OOBINLINE;
	    	    	break;
	    	    	
			 	case IDC_RCVBUF:
			 		nOptName = SO_RCVBUF;
	    	    	break;
	    	    	
			 	case IDC_REUSEADDR:
			 		nOptName = SO_REUSEADDR;
	    	    	break;
	    	    	
			 	case IDC_SNDBUF:
			 		nOptName = SO_SNDBUF;
	    	    	break;
	    	    	
			 	case IDC_TYPE:
			 		nOptName = SO_TYPE;
	    	    	break;
	    	    	
			 	case IDC_TCPNODELAY:
 					SetDlgItemText (hDlg, IDC_LEVELIN, "IPPROTO_TCP");
			 		nOptName = TCP_NODELAY;
	    	    	return TRUE;
	    	    	
	    	    case IDC_SOCKIN:
	    	    case IDC_LEVELIN:
	    	    case IDC_OPTVAL:
		            SetDlgItemText (hDlg, IDC_LEVELIN, "SOL_SOCKET");
			 		return TRUE;
	    	}
		return TRUE;
    }        
    return FALSE;
    
} /* end WSSockOptDlgProc() */

/*---------------------------------------------------------------------
 * Function: WSOobOptDlgProc()
 *
 * Description:
 */                                        
BOOL CALLBACK WSOobOptDlgProc (
	HWND hDlg,
	UINT msg,
	UINT wParam,
	LPARAM lParam)
{
	static BOOL bOobSend, bOobPoll, bOobInLine, bOobOutSound, bOobInSound;
	static HANDLE hwnd, hInst;
	
   switch (msg) {
        case WM_INITDIALOG:
            
            /* get parameters passed */
    		if (lParam) {				
		    	hInst = LOWORD(lParam);
		    	hwnd  = HIWORD(lParam);
		    }
                                              
            /* init local work variables from actual values */                                           
			bOobSend     = (stWSAppData.nOptions & OPTION_OOBSEND);
			bOobPoll     = (stWSAppData.nOptions & OPTION_OOBPOLL);
			bOobInLine   = (stWSAppData.nOptions & OPTION_OOBINLINE);
			bOobOutSound = (stWSAppData.nOptions & OPTION_OOBOUTSOUND);
			bOobInSound  = (stWSAppData.nOptions & OPTION_OOBINSOUND);

			/* set display values */
			CheckDlgButton (hDlg, IDC_OOBSEND, bOobSend);
			CheckDlgButton (hDlg, IDC_OOBPOLL, bOobPoll);
			CheckDlgButton (hDlg, IDC_OOBINLINE, bOobInLine);			
 			CheckDlgButton (hDlg, IDC_OOBOUTSOUND, bOobOutSound);
			CheckDlgButton (hDlg, IDC_OOBINSOUND, bOobInSound);
			SetDlgItemInt(hDlg, IDC_OOBSENDAMNT, stWSAppData.nOobOutLen, FALSE);
			SetDlgItemInt(hDlg, IDC_OOBRECVAMNT, stWSAppData.nOobInLen, FALSE);
			SetDlgItemInt(hDlg, IDC_OOBSENDINTRVL, stWSAppData.nOobInterval, FALSE);
		
			SetFocus (GetDlgItem (hDlg, IDOK));
            CenterWnd(hDlg, hwnd, TRUE);

	    	return FALSE;

		case WM_COMMAND:
	    	switch (wParam) {
	    	
	    		case IDCANCEL:
	    		
	    			/* "Cancel" button pressed, so just leave (ignore any new values) */
		            EndDialog (hDlg, FALSE);
	    	        return TRUE;
	    	        
	        	case IDOK:
	        		/* "OK" button pressed, so get the new values (if there are any) */

	        		/* If socket available, call setsockopt(SO_OOBINLINE) to
	        		 *  set option to selected setting (enabled or disabled) */
					if (stWSAppData.nSockState != STATE_NONE) {
						set_oobinline (hInst, hwnd, bOobInLine);
					}
	        		
	        		stWSAppData.nOptions |= OPTION_OOBSEND;
	        		stWSAppData.nOptions &= (bOobSend ? OPTION_MASK : ~OPTION_OOBSEND);
	        		
	        		stWSAppData.nOptions |= OPTION_OOBPOLL;
	        		stWSAppData.nOptions &= (bOobPoll ? OPTION_MASK : ~OPTION_OOBPOLL);
	        		
	        		stWSAppData.nOptions |= OPTION_OOBINLINE;
	        		stWSAppData.nOptions &= (bOobInLine ? OPTION_MASK : ~OPTION_OOBINLINE);
	        		
	        		stWSAppData.nOptions |= OPTION_OOBOUTSOUND;
	        		stWSAppData.nOptions &= (bOobOutSound ? OPTION_MASK : ~OPTION_OOBOUTSOUND);
	        		
	        		stWSAppData.nOptions |= OPTION_OOBINSOUND;
	        		stWSAppData.nOptions &= (bOobInSound ? OPTION_MASK : ~OPTION_OOBINSOUND);
	        			
	        		stWSAppData.nOobOutLen = GetDlgItemInt (hDlg, IDC_OOBSENDAMNT, NULL, FALSE);
	        		stWSAppData.nOobInLen  = GetDlgItemInt (hDlg, IDC_OOBRECVAMNT, NULL, FALSE);
	        		stWSAppData.nOobInterval = GetDlgItemInt (hDlg, IDC_OOBSENDINTRVL, NULL, FALSE);
	        		
		            EndDialog (hDlg, TRUE);
	    	        return TRUE;
	    	        
	    	    case IDC_OOBSEND:
	    	    	bOobSend = !bOobSend;
//					CheckDlgButton (hDlg, IDC_OOBSEND, bOobSend);
	    	    	return TRUE;
	    	    	
	    	    case IDC_OOBPOLL:
	    	    	bOobPoll = !bOobPoll;
//					CheckDlgButton (hDlg, IDC_OOBPOLL, bOobPoll);
	    	    	return TRUE;
	    	    	
			 	case IDC_OOBINLINE:
			 		bOobInLine = !bOobInLine;
//			 		CheckDlgButton (hDlg, IDC_OOBINLINE, bOobInLine);
	    	    	return TRUE;
	    	    	
	    	    case IDC_OOBOUTSOUND:
	    	    	bOobOutSound = !bOobOutSound;
	    	    	return TRUE;
	    	    	
			 	case IDC_OOBINSOUND:
			 		bOobInSound = !bOobInSound;
	    	    	return TRUE;
	    	    	
	    	    case IDC_OOBSENDAMNT:
	    	    case IDC_OOBRECVAMNT:
	    	    case IDC_OOBSENDINTRVL:
			 		return TRUE;
	    	}
 		break;
    }        
    return FALSE;
    
} /* end WSOobOptDlgProc() */

/*---------------------------------------------------------------------
 * Function: WSSocketDlgProc()
 *
 * Description:
 */
BOOL CALLBACK WSSocketDlgProc (
	HWND hDlg,
	UINT msg,
	UINT wParam,
	LONG lParam)
{
	static int nProtocol;
	static HANDLE hwnd, hInst;
	lParam = lParam;	/* avoid warning */

    switch (msg) {
        case WM_INITDIALOG:

			/* get parameters passed */
    		if (lParam) {				
		    	hInst = LOWORD(lParam);
		    	hwnd  = HIWORD(lParam);
		    }

           	/* figure out what socket type to check (if selected already) */
       		switch (stWSAppData.nProtocol) {
       			case PROT_DG:
       				nProtocol = IDC_SOCKDG;
       				break;
       			case PROT_DS:
       			default:
       				nProtocol = IDC_SOCKDS;
       		}
       		CheckRadioButton (hDlg, IDC_SOCKDG, IDC_SOCKDS, nProtocol);
            CenterWnd(hDlg, hwnd, TRUE);

	    	return FALSE;
	    
		case WM_COMMAND:
	    	switch (wParam) {
	    		case IDC_SOCKDG:
	    			nProtocol = IDC_SOCKDG;
	    			return TRUE;
	    			
	    		case IDC_SOCKDS:
	    			nProtocol = IDC_SOCKDS;
	    			return TRUE;
	    			
	        	case IDOK:
					/* Get the selected socket, update state and return */	        	
					stWSAppData.nSock = socket (AF_INET, 
						nProtocol == IDC_SOCKDS ? SOCK_STREAM : SOCK_DGRAM, 
						0);
					if (stWSAppData.nSock == INVALID_SOCKET)  {
        				WSAperror(WSAGetLastError(), "socket()", hInst);
					} else {
	    				/*---------------------------------*/                                        
						stWSAppData.nSockState = STATE_OPEN;
    					/*---------------------------------*/
    				}
	        		/* fall through */
	        		
	    		case IDCANCEL:
	    			/* Save the socket type already selected */
					switch (nProtocol) {
						case IDC_DATAGRAM:
							stWSAppData.nProtocol = PROT_DG;
						case IDC_DATASTREAM:
							stWSAppData.nProtocol = PROT_DS;
					}	    			
	    		
	            	EndDialog (hDlg, 0);
	            	return TRUE;
	    	}
		break;
    }
    return FALSE;
} /* end WSSocketDlgProc() */

