/*--------------------------------------------------------------------------- 
 *  Program: WAL.EXE  WinSock Application Launcher
 *
 *  filename: blocking.c
 *                                                   
 *  copyright by Bob Quinn, 1995
 *
 *  Description:
 *   This module contains the Windows Sockets network code in a
 *   sample appliction that does blocking calls.
 *
 *  This software is not subject to any  export  provision  of
 *  the  United  States  Department  of  Commerce,  and may be
 *  exported to any country or planet.
 *
 *  Permission is granted to anyone to use this  software  for any  
 *  purpose  on  any computer system, and to alter it and redistribute 
 *  it freely, subject to the following  restrictions:
 *
 *  1. The  author is not responsible for the consequences of
 *     use of this software, no matter how awful, even if they
 *     arise from flaws in it.
 *
 *  2. The origin of this software must not be misrepresented,
 *     either by explicit claim or by omission.  Since few users
 *     ever  read  sources, credits must appear in the documenta-
 *     tion.
 *
 *  3. Altered versions must be plainly marked as such, and
 *     must not be misrepresented as being the original software.
 *     Since few users ever read sources, credits must appear  in
 *     the documentation.
 *
 *  4. This notice may not be removed or altered.
 *	 
 ---------------------------------------------------------------------------*/
#include <windows.h>
#include <windowsx.h>
#include <winsock.h> /* Windows Sockets */
#include <string.h>  /* for _fxxxxx() functions */ 

#include "resource.h"
#include "..\winsockx.h"
#include "wal.h"

#undef BUFSIZE
#define BUFSIZE DIX_MSS

/*---------------------------------------------------------------------------
 * Function: bl_ds_cn()
 *
 * Description:
 *  Blocking datastream connect
 *
 */
int bl_ds_cn (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;	/* Socket structure */
	int	 wRet = SOCKET_ERROR;		/* work variable */

	/* Get a TCP socket */
	if (stWSAppData.nSockState == STATE_NONE) {
		stWSAppData.nSock = socket (AF_INET, SOCK_STREAM, 0);
		if (stWSAppData.nSock == INVALID_SOCKET)  {
        	WSAperror(WSAGetLastError(), "socket()", hInst);
        	goto AppExit;
        }
		/*---------------------------------*/
		stWSAppData.nSockState = STATE_OPEN;
		/*---------------------------------*/
	}

    /* Enable OOBInLine option if user asked to enable it */    
    if (stWSAppData.nOptions & OPTION_OOBINLINE) {                                        
		set_oobinline (hInst, hwnd, TRUE);
	}

	/* Initialize the Sockets Internet Address (sockaddr_in) structure */
	_fmemset ((LPSTR)&(stLclAddr), 0, sizeof(struct sockaddr_in)); 

	/* Get IP Address from a hostname (or addr string in dot-notation) */
	stLclAddr.sin_addr.s_addr = GetAddr(stWSAppData.szHost); 

	/* convert port number from host byte order to network byte order */
	stLclAddr.sin_port = htons (stWSAppData.nPortNumber);	
	stLclAddr.sin_family = PF_INET;	/* Internet Address Family */
                                             
	/*----------------------------------------*/
	stWSAppData.nSockState = STATE_CONN_PENDING;
	/*----------------------------------------*/
                                               
	wRet = connect(stWSAppData.nSock,
		(struct sockaddr FAR*)&stLclAddr, 
		sizeof(struct sockaddr_in)); 
	if (wRet == SOCKET_ERROR) {
		bl_close(hInst, hwnd);
		WSAperror(WSAGetLastError(), "connect()", hInst);
		goto AppExit;
	}
	/*-------------------------------------*/
	stWSAppData.nSockState = STATE_CONNECTED;
	/*-------------------------------------*/

    /* get timers for statistics updates & I/O (if requested) */                        
	get_timers(hInst, hwnd);
      
	/* Tell ourselves to do next step! */
	PostMessage(hwnd, WM_COMMAND, anIoCmd[stWSAppData.nIoMode], 0L);

AppExit:    
	return (wRet);

} /* end bl_ds_cn() */

/*---------------------------------------------------------------------------
 * Function: bl_ds_ls()
 *
 * Description:
 *  Blocking datastream listen/accept
 *
 */
int bl_ds_ls (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;		/* Socket structures */
	struct sockaddr_in stRmtAddr;
	int	 wRet = SOCKET_ERROR;		/* work variable */
	int	 nAddrLen, nWSAerror;
	SOCKET hNewSock;

	/* Get a TCP socket */
	if (stWSAppData.nSockState == STATE_NONE) {
		stWSAppData.nSock = socket (AF_INET, SOCK_STREAM, 0);
		if (stWSAppData.nSock == INVALID_SOCKET)  {
			WSAperror(WSAGetLastError(), "socket()", hInst);
			goto AppExit;
		}
		/*---------------------------------*/
		stWSAppData.nSockState = STATE_OPEN;
		/*---------------------------------*/
	}

    /* Enable OOBInLine option if user asked to enable it */    
    if (stWSAppData.nOptions & OPTION_OOBINLINE) {                                        
		set_oobinline (hInst, hwnd, TRUE);
	}

	/* convert port number from host byte order to network byte order */
	stLclAddr.sin_port = htons (stWSAppData.nPortNumber);
	stLclAddr.sin_addr.s_addr = 0L;	/* no restriction on address */
	stLclAddr.sin_family = PF_INET;	/* Internet Address Family */

	/* Name the local socket */
	wRet = bind(stWSAppData.nSock,
		(struct sockaddr FAR*)&stLclAddr, 
		sizeof(struct sockaddr_in)); 
	if (wRet == SOCKET_ERROR) {
		bl_close(hInst, hwnd);
		WSAperror(WSAGetLastError(), "bind()", hInst);
		goto AppExit;
	}
	/*---------------------------------*/
	stWSAppData.nSockState = STATE_BOUND;
	/*---------------------------------*/

	/* Begin listening for incoming connections on socket */  
	wRet = listen(stWSAppData.nSock, 1);  
	if (wRet == SOCKET_ERROR) {
		bl_close(hInst, hwnd);
		WSAperror(WSAGetLastError(),"listen()", hInst);
		goto AppExit;
	}
	/*-------------------------------------*/
	stWSAppData.nSockState = STATE_LISTENING;
	/*-------------------------------------*/
	
	do_stats(hwnd, hInst, FALSE);	/* update window to show current state */
  
	/* Initialize the Sockets Internet Address (sockaddr_in) structure */
	_fmemset ((LPSTR)&(stRmtAddr), 0, sizeof(struct sockaddr_in)); 
                 
	nAddrLen = sizeof(struct sockaddr_in);
	hNewSock = accept(stWSAppData.nSock,
		(struct sockaddr FAR*)&stRmtAddr, 
		(int FAR*)&nAddrLen); 
	if (hNewSock == INVALID_SOCKET) {
    	nWSAerror = WSAGetLastError();
		    	
	    /* if blocking call canceled then close, else report error */
	    if (nWSAerror == WSAEINTR) {
	    	 bl_close(hInst, hwnd);
	    } else {
			bl_close(hInst, hwnd);
   			WSAperror(nWSAerror, "accept()", hInst);
		}
		wRet = SOCKET_ERROR;
		goto AppExit;
	}
  
	/*-------------------------------------*/
	stWSAppData.nSockState = STATE_CONNECTED;
	/*-------------------------------------*/ 
	
	/* This example is a (very) limited server that can only 
	 *  handle one connection at a time.  So, we close the 
	 * listening socket now and save the newly accepted one */
	wRet = bl_close (hInst, hwnd);	/* close listening socket */
	stWSAppData.nSock = hNewSock;	/* save new (accept) socket */
                                                
    /* get timers for statistics updates & I/O (if requested) */                        
	get_timers(hInst, hwnd);

	/* Tell ourselves to do next step! */
	PostMessage(hwnd, WM_COMMAND, anIoCmd[stWSAppData.nIoMode], 0L);

AppExit:    
	return (wRet);

} /* end bl_ds_ls() */

/*---------------------------------------------------------------------------
 * Function: bl_dg_cn()
 *
 * Description:
 *  Blocking datagram "connect".
 *
 */
int bl_dg_cn (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;		/* Socket structure */
	int	 wRet = SOCKET_ERROR;		/* work variable */
	int nWSAerror;

	/* Get a UDP socket */
	if (stWSAppData.nSockState == STATE_NONE) {
		stWSAppData.nSock = socket (AF_INET, SOCK_DGRAM, 0);
		if (stWSAppData.nSock == INVALID_SOCKET)  {
        	WSAperror(WSAGetLastError(), "socket()", hInst);
        	goto AppExit;
        }
		/*--------------------------------*/
		stWSAppData.nSockState = STATE_OPEN;
		/*--------------------------------*/
	}

	/* Initialize the Sockets Internet Address (sockaddr_in) structure */
	_fmemset ((LPSTR)&(stLclAddr), 0, sizeof(struct sockaddr_in)); 

	/* Get IP Address from a hostname (or IP address string in dot-notation) */
	stLclAddr.sin_addr.s_addr = GetAddr(stWSAppData.szHost);

	/* convert port number from host byte order to network byte order */
	stLclAddr.sin_port = htons (stWSAppData.nPortNumber);	
	stLclAddr.sin_family = PF_INET;	/* Internet Address Family */
                                             
	/*----------------------------------------*/
	stWSAppData.nSockState = STATE_CONN_PENDING;
	/*----------------------------------------*/
                                               
	wRet = connect(stWSAppData.nSock,
		(struct sockaddr FAR*)&stLclAddr, 
		sizeof(struct sockaddr_in)); 
	if (wRet == SOCKET_ERROR) {
		nWSAerror = WSAGetLastError();
		    	
	    /* if blocking call canceled then close, else report error */
	    if (nWSAerror == WSAEINTR) {
	     	bl_close(hInst, hwnd);
	    } else {
			bl_close(hInst, hwnd);
        	WSAperror(nWSAerror, "connect()", hInst);
		}
		goto AppExit;
	}
      
	/*-------------------------------------*/
	stWSAppData.nSockState = STATE_CONNECTED;
	/*-------------------------------------*/

    /* get timers for statistics updates & I/O (if requested) */
	get_timers(hInst, hwnd);

	/* Tell ourselves to do next step! */
	PostMessage(hwnd, WM_COMMAND, anIoCmd[stWSAppData.nIoMode], 0L);

AppExit:    
	return (wRet);

} /* end bl_dg_cn() */

/*---------------------------------------------------------------------------
 * Function: bl_dg_ls()
 *
 * Description:
 *  Blocking datagram "listen".
 *
 */
int bl_dg_ls (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;		/* Socket structure */
	int	 wRet = SOCKET_ERROR;		/* work variable */

	/* Get a UDP socket */
	if (stWSAppData.nSockState == STATE_NONE) {
		stWSAppData.nSock = socket (AF_INET, SOCK_DGRAM, 0);
		if (stWSAppData.nSock == INVALID_SOCKET)  {
			WSAperror(WSAGetLastError(), "socket()", hInst);
        	goto AppExit;
        }
		/*---------------------------------*/
		stWSAppData.nSockState = STATE_OPEN;
		/*---------------------------------*/
	}

	/* convert port number from host byte order to network byte order */
	stLclAddr.sin_port = htons (stWSAppData.nPortNumber);	
	stLclAddr.sin_addr.s_addr = 0L;	/* don't restrict incoming address */
	stLclAddr.sin_family = PF_INET;	/* Internet Address Family */
                                              
	wRet = bind(stWSAppData.nSock,
			(struct sockaddr FAR*)&stLclAddr, 
			sizeof(struct sockaddr_in)); 
	if (wRet == SOCKET_ERROR) {
		bl_close(hInst, hwnd);
		WSAperror(WSAGetLastError(), "bind()", hInst);
		goto AppExit;
	}
      
	/*---------------------------------*/
	stWSAppData.nSockState = STATE_BOUND;
	/*---------------------------------*/

	do_stats(hwnd, hInst, FALSE);	/* update window to show current state */

	/* Tell ourselves to do next step! */
	PostMessage(hwnd, WM_COMMAND, anIoCmd[stWSAppData.nIoMode], 0L);

AppExit:    
	return (wRet);

} /* end bl_dg_ls */

/*---------------------------------------------------------------------------
 * Function: bl_w_r()
 *
 * Description:
 *   Blocking write and read.  Our CLIENTS connecting to ECHO port use this.
 */
int bl_w_r(HANDLE hInst, HANDLE hwnd)
{
	int		cbInLen;                /* Bytes in Input Buffer */
	int		cbOutLen;               /* Bytes in Output Buffer */
	int		cbBufSize;				/* Length of I/O */
	int		i, wRet, nWSAerror;
	static  bBusy=FALSE;            /* busy flag */
	
	/* set busy flag to prevent overlapping loops */
    if (!bBusy)
      bBusy=TRUE;
    else
      return(0);
                                                       
	cbBufSize = stWSAppData.nLength;

	if (stWSAppData.nOptions & OPTION_SOUND)
		MessageBeep(0xFFFF);
		
	for (i=0; i<stWSAppData.nLoopLimit; i++, stWSAppData.wOffset++) {

	    /* adjust offset into output string buffer */
	    if (stWSAppData.wOffset >= cbBufSize) {
	    	stWSAppData.wOffset = 0;
	    }
	     
	    /* write to server */
	    cbOutLen = 0;
	    while (cbOutLen < cbBufSize) {
		    wRet = send (stWSAppData.nSock, 
		    	(LPSTR)&(achChargenBuf[stWSAppData.wOffset+cbOutLen]),
		    	(cbBufSize-cbOutLen), 0);
		    if (wRet == SOCKET_ERROR) {
		    	nWSAerror = WSAGetLastError();
		    	
	    		/* if blocking call canceled then close, else report error */
	    		if (nWSAerror == WSAEINTR) {
	    		 	bl_close(hInst, hwnd);
	    		} else {
	    		    bl_close(hInst, hwnd);
        			WSAperror(nWSAerror, "send()", hInst);
				}
		    } else {
			/* Tally amount sent, to see how much left to send */
		    	cbOutLen += wRet;
		    	stWSAppData.lBytesOut += wRet;
		    }
	    } /* end send() loop */

	    /* read from server (hopefully what we just wrote) */
	    cbInLen = 0;
	    while (cbInLen < cbBufSize) {
		    wRet = recv (stWSAppData.nSock, (LPSTR)achInBuf, 
			    (BUF_SIZE-cbInLen), 0);
		    if (wRet == SOCKET_ERROR) {
		    	nWSAerror = WSAGetLastError();
		    	
	    		/* if blocking call canceled then close, else report error */
	    		if (nWSAerror == WSAEINTR) {
	    		 	bl_close(hInst, hwnd);
	    		} else {
	    		    bl_close(hInst, hwnd);
        			WSAperror(nWSAerror, "recv()", hInst);
				}
        		goto wr_end;
		    } else if (wRet == 0) { /* Other side closed socket */
       			MessageBox (hwnd, achOutBuf, 
					"server closed connection", MB_OK);
			    goto wr_end;
		    } else {
		    	cbInLen += wRet;
		    	stWSAppData.lBytesIn += wRet;
		    }
        } /* end recv() loop */
        
	} /* end main loop */
   
wr_end:
	            
	/* Tell ourselves to do it again (if we're not 
	 *  using a timer and we're still connected)! */
	if (!stWSAppData.nWinTimer && 
	   (stWSAppData.nSockState == STATE_CONNECTED))
		PostMessage(hwnd, WM_COMMAND, IDM_WRITE_READ, 0L);

    bBusy=FALSE;  /* reset busy flag */
    
	return (cbOutLen); 
} /* end bl_w_r() */

/*---------------------------------------------------------------------------
 * Function: bl_r_w()
 *
 * Description:
 *   Blocking read and write.  Our SERVERS providing ECHO service use this.
 */
int bl_r_w(HANDLE hInst, HANDLE hwnd)
{
	int		cbInLen;                /* Bytes in Input Buffer */
	int		cbOutLen;               /* Bytes in Output Buffer */
	int		cbBufSize;				/* Length of I/O */
	int		i, wRet, nWSAerror;
                                                       
	cbBufSize = stWSAppData.nLength;

	if (stWSAppData.nOptions & OPTION_SOUND)
		MessageBeep(0xFFFF);
		
	for (i=0; i<stWSAppData.nLoopLimit; i++, stWSAppData.wOffset++) {
	     
	    /* read from client */
	    cbInLen = 0;
	    while (cbInLen < cbBufSize) {
		    wRet = recv (stWSAppData.nSock, 
			    (LPSTR)achInBuf, (cbBufSize-cbInLen), 0);
		    if (wRet == SOCKET_ERROR) {
		    	nWSAerror = WSAGetLastError();
		    	
	    		/* if blocking call canceled then close, else report error */
	    		if (nWSAerror == WSAEINTR) {
	    		 	bl_close(hInst, hwnd);
	    		} else {
					bl_close(hInst, hwnd);
        			WSAperror(nWSAerror, "recv()", hInst);
				}
 				goto rw_end;
		    } else if (wRet == 0) { /* Other side closed socket */
       			MessageBox (hwnd, achOutBuf, 
					"client closed connection", MB_OK);
				goto rw_end;
		    } else {
		    	cbInLen += wRet;
		    	stWSAppData.lBytesIn += wRet;
		    }
        } /* end recv() loop */

	    /* write to client */
	    cbOutLen = 0;
	    while (cbOutLen < cbBufSize) {
		    wRet = send (stWSAppData.nSock, 
		    	(LPSTR)&(achInBuf[cbOutLen]), 
		    	(cbBufSize-cbOutLen), 0);
		    if (wRet == SOCKET_ERROR) {
		    	nWSAerror = WSAGetLastError();
		    	
	    		/* if blocking call canceled then close, else report error */
	    		if (nWSAerror == WSAEINTR) {
	    		 	bl_close(hInst, hwnd);
	    		} else {
					bl_close(hInst, hwnd);
        			WSAperror(nWSAerror, "send()", hInst);
				}
        		goto rw_end;
        	} else {
			/* Tally amount sent, to see how much left to send */
		    	cbOutLen += wRet;
		    	stWSAppData.lBytesOut += wRet;
		    }
	    } /* end send() loop */
        
	} /* end main loop */
   
	/* Datagram Servers aren't "connected" until data arrives, 
	 *  so change it now (and start timer) */
	if (stWSAppData.nSockState != STATE_CONNECTED) {
		
        /*-------------------------------------*/
	    stWSAppData.nSockState = STATE_CONNECTED;
	    /*-------------------------------------*/
		
    	/* get timers for statistics updates & I/O (if requested) */
		get_timers(hInst, hwnd);
	}
		
rw_end:
	            
	/* Tell ourselves to do it again (if we're not 
	 *  using a timer and we're still connected)! */
	if (!stWSAppData.nWinTimer && 
	    (stWSAppData.nSockState == STATE_CONNECTED)) {
		PostMessage(hwnd, WM_COMMAND, IDM_READ_WRITE, 0L);
	}

	return (cbOutLen); 
} /* end bl_r_w() */

/*---------------------------------------------------------------------------
 * Function: bl_r()
 *
 * Description:
 *   Blocking read
 */
int bl_r(HANDLE hInst, HANDLE hwnd)
{
	int		cbInLen=0;              /* Bytes in Input Buffer */
	int		cbBufSize;				/* Length of I/O */
	char	achInBuf  [BUF_SIZE];   /* Input Buffer */
	int		i, wRet, nWSAerror;
                                                       
	cbBufSize = stWSAppData.nLength;

	if (stWSAppData.nOptions & OPTION_SOUND)
		MessageBeep(0xFFFF);
		
	for (i=0; i<stWSAppData.nLoopLimit; i++, stWSAppData.wOffset++) {

	    /* read from server */
	    cbInLen = 0;
	    while (cbInLen < cbBufSize) {
		    wRet = recv (stWSAppData.nSock, 
			    (LPSTR)achInBuf, 
			    (cbBufSize-cbInLen), 0);
		    if (wRet == SOCKET_ERROR) {
		    	nWSAerror = WSAGetLastError();
		    	
	    		/* if blocking call canceled then close, else report error */
	    		if (nWSAerror == WSAEINTR) {
	    		 	bl_close(hInst, hwnd);
	    		} else {
					bl_close(hInst, hwnd);
       				WSAperror(nWSAerror, "recv()", hInst);
				}
        		goto r_end;
		    } else if (wRet == 0) { /* Other side closed socket */
       			MessageBox (hwnd, achOutBuf, 
					"server closed connection", MB_OK);
				goto r_end;;
		    } else {
			    /* tally amount received */
			    cbInLen += wRet;
			    stWSAppData.lBytesIn += wRet;
		    }
	    } /* end recv() loop */
        
	} /* end main loop */

	/* Datagram Servers aren't "connected" until data arrives, 
	 *  so change it now (and start timer) */
	if (stWSAppData.nSockState != STATE_CONNECTED) {
	
        /*-------------------------------------*/
	    stWSAppData.nSockState = STATE_CONNECTED;
	    /*-------------------------------------*/
		
	    /* get timers for statistics updates & I/O (if requested) */
		get_timers(hInst, hwnd);
	}

r_end:	            
	/* Tell ourselves to do it again (if we're not 
	 *  using a timer and we're still connected)! */
	if (!stWSAppData.nWinTimer && 
	   (stWSAppData.nSockState == STATE_CONNECTED))
		PostMessage(hwnd, WM_COMMAND, IDM_READ, 0L);

	return (cbInLen); 
} /* end bl_r() */

/*---------------------------------------------------------------------------
 * Function: bl_w()
 *
 * Description:
 *   Blocking write
 */
int bl_w(HANDLE hInst, HANDLE hwnd)
{
	int		cbOutLen=0;             /* Bytes in Output Buffer */
	int		cbBufSize;				/* Length of I/O */
	int		i, wRet, nWSAerror;
                                                       
	cbBufSize = stWSAppData.nLength;
	
	if (stWSAppData.nOptions & OPTION_SOUND)
		MessageBeep(0xFFFF);
		
	for (i=0; i<stWSAppData.nLoopLimit; i++, stWSAppData.wOffset++) {

		/* adjust offset into output string buffer */
		if (stWSAppData.wOffset >= cbBufSize) {  
			stWSAppData.wOffset = 0;
		}
	     
		/* write to server */
		cbOutLen = 0;
		while (cbOutLen < cbBufSize) {
		    wRet = send (stWSAppData.nSock, 
		    	(LPSTR)&(achChargenBuf[stWSAppData.wOffset+cbOutLen]),
		    	(cbBufSize-cbOutLen), 0);
		    if (wRet == SOCKET_ERROR) {
		    	nWSAerror = WSAGetLastError();
		    	
	    		/* if blocking call canceled then close, else report error */
	    		if (nWSAerror == WSAEINTR) {
	    		 	bl_close(hInst, hwnd);
	    		} else {
					bl_close(hInst, hwnd);
        			WSAperror(nWSAerror, "send()", hInst);
				}
       			goto w_end;
		    } else {
			    /* Tally amount sent */
			    cbOutLen += wRet;
			    stWSAppData.lBytesOut += wRet;
		    }
	    } /* end send() loop */
	} /* end main loop */

w_end:	            
	/* Tell ourselves to do it again (if we're not 
	 *  using a timer and we're still connected)! */
	if (!stWSAppData.nWinTimer && 
	   (stWSAppData.nSockState == STATE_CONNECTED))
		PostMessage(hwnd, WM_COMMAND, IDM_WRITE, 0L);

	return (cbOutLen); 
} /* end bl_w() */

/*---------------------------------------------------------------------------
 * Function: bl_close()
 *
 * Description:
 *  Differences between this and a non-blocking close is that this cannot 
 *  handle an WSAEWOULDBLOCK error gracefully (i.e. ignore it ), and this  
 *  will* handle WSAEINPROGRESS gracefully.
 */
int bl_close (HANDLE hInst, HANDLE hwnd) 
{   
	int wRet=0;
	
	/* we may have already set this state earlier, 
	 *  but not in all cases */
	/*-----------------------------------------*/
	stWSAppData.nSockState = STATE_CLOSE_PENDING;	
	/*-----------------------------------------*/

	if (WSAIsBlocking()) {
		/* Cancel pending blocking operation before we try to close 
		 *  (which would just fail with WSAEINPROGRESS).  When the
		 *  pending blocking operation fails with WSAEINTR it will
		 *  call bl_close() and close then close will proceed */
		WSACancelBlockingCall();
	} else {
        /* Doing a close without calling shutdown(how=1), and looping on
         *  recv() first--the way we do in CloseConn() in WinSockX.lib--
         *  is dangerous and not robust ...but we're experimenting here */
    	wRet = closesocket(stWSAppData.nSock);
    	if (wRet == SOCKET_ERROR) {
    		if (IsWindow(hwnd)) {
       			WSAperror(WSAGetLastError(), "closesocket()", hInst);
			}
      	}
 	    /* Socket is invalid now (if it wasn't already) */
		/*---------------------------------*/
    	stWSAppData.nSockState = STATE_NONE;
		/*---------------------------------*/

		/* free the timer resource(s) (if Window not destroyed yet) */
		if (IsWindow (hwnd)) {
			KillTimer (hwnd, STATS_TIMER_ID);
			if (stWSAppData.nWinTimer)
				KillTimer (hwnd, APP_TIMER_ID);
		}
    } /* end else */
    
	return (wRet);
}  /* end bl_close() */

