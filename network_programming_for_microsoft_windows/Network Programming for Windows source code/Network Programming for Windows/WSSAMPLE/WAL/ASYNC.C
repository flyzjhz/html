/*---------------------------------------------------------------------------
 *  Program: WAL.EXE  WinSock Application Launcher
 *
 *  filename: async.c
 *
 *  copyright by Bob Quinn, 1995
 *
 *  Description:
 *   This module contains the Windows Sockets network code in a
 *   sample application to uses asynchronous notification for I/O.
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
 ----------------------------------------------------------------------------*/
#include <windows.h>
#include <windowsx.h> /* for 32-bit _fmemset */
#include <winsock.h>  /* Windows Sockets */
#include <string.h>   /* for _fxxxxx() functions */ 

#include "resource.h"
#include "..\winsockx.h"
#include "wal.h"


extern int recv_count;	/* number of recv() calls */
/*---------------------------------------------------------------------------
 * Function: as_ds_cn()
 *
 * Description:
 *  Asynch datastream connect.
 *
 */
int as_ds_cn (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;	/* Socket structure */
	int	wRet = SOCKET_ERROR;		/* work variables */
	BOOL bInProgress = FALSE;
	int nWSAerror;

	/* Get a TCP socket */
	stWSAppData.nSock = socket (AF_INET, SOCK_STREAM, 0);
	if (stWSAppData.nSock == INVALID_SOCKET)  {
        WSAperror(WSAGetLastError(), "socket()", hInst);
        goto AppExit;
	}
    /*---------------------------------*/                                        
	stWSAppData.nSockState = STATE_OPEN;
    /*---------------------------------*/                                        

    /* Enable OOBInLine option if user asked to enable it */    
    if (stWSAppData.nOptions & OPTION_OOBINLINE) {
    		set_oobinline (hInst, hwnd, TRUE);
	}

	/* Initialize the Sockets Internet Address (sockaddr_in) structure */
	_fmemset ((LPSTR)&(stLclAddr), 0, sizeof(struct sockaddr_in)); 

	/* Get IP Address from a hostname (or addr string in dot-notation) */
	stLclAddr.sin_addr.s_addr = GetAddr(stWSAppData.szHost);

	/* register for asynchronous notification of all events
	 *  this socket could possibly get.(note: WSAAsyncSelect()
	 *  will also make the socket non-blocking) */
	wRet = WSAAsyncSelect(stWSAppData.nSock, hwnd, IDM_ASYNC,
		(FD_READ | FD_WRITE | FD_OOB | FD_CONNECT | FD_CLOSE));
	if (wRet == SOCKET_ERROR) {
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(), "WSAAsyncSelect()", hInst);
	}

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
		if (nWSAerror != WSAEWOULDBLOCK) {
	    	nb_close (hInst, hwnd);
			WSAperror(nWSAerror, "connect()", hInst);
			goto AppExit;
		}
	}
                            
    /* get timers for statistics updates & I/O (if requested) */                        
	get_timers(hInst, hwnd);

AppExit:    
	return (wRet);

} /* end as_ds_cn() */

/*---------------------------------------------------------------------------
 * Function: as_ds_ls()
 *
 * Description:
 *  Asynch datastream listen/accept for a TCP server
 *
 */
int as_ds_ls (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;	/* Socket structures */
	int	wRet = SOCKET_ERROR;	/* work variables */

	/* Get a TCP socket */
	stWSAppData.nSock = socket (AF_INET, SOCK_STREAM, 0);
	if (stWSAppData.nSock == INVALID_SOCKET)  {
		WSAperror(WSAGetLastError(), "socket()", hInst);
		goto AppExit;
	}
	/*---------------------------------*/
	stWSAppData.nSockState = STATE_OPEN;
	/*---------------------------------*/

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
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(), "bind()", hInst);
		goto AppExit;
	}
	/*---------------------------------*/
	stWSAppData.nSockState = STATE_BOUND;
	/*---------------------------------*/

	/* register for asynchronous notification of all events
	 *  this socket could possibly get.(note: WSAAsyncSelect()
	 *  will also make the socket non-blocking) */
	wRet = WSAAsyncSelect(stWSAppData.nSock, hwnd, IDM_ASYNC,
		(FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE));
	if (wRet == SOCKET_ERROR) {
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(),"WSAAsyncSelect()", hInst);
	}

	/* Begin listening for incoming connections on socket */  
	wRet = listen(stWSAppData.nSock, 1);  
	if (wRet == SOCKET_ERROR) {
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(), "listen()", hInst);
		goto AppExit;
	}
	/*-------------------------------------*/
	stWSAppData.nSockState = STATE_LISTENING;
	/*-------------------------------------*/
	
	do_stats(hwnd, hInst, FALSE);	/* update window to show current state */
	
AppExit:    
	return (wRet);

} /* end as_ds_ls() */

/*---------------------------------------------------------------------------
 * Function: as_accept()
 *
 * Description:
 *   accept an incoming connection request (called after application
 *    receives an FD_ACCEPT asynchronous event notification message.
 */
int as_accept(HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stRmtAddr;
	SOCKET hNewSock;	/* new socket returned from accept() */
	int	 nAddrLen;		/* for length of address structure */	
	int nWSAerror;
	int wRet;
	
	/* Note: even though we expect accept() succeed immediately, it
	 *  could fail with "would block".  To protect, we loop on the
	 *  "would block" error.  Even in this simple loop with success
	 *  almost guaranteed, we need to have a blocking hook (yield) 
	 *  in this loop to keep from locking-up Windows 3.x. */
	do {                 
		/* Initialize the Sockets Internet Address (sockaddr_in) struct */
		_fmemset ((LPSTR)&(stRmtAddr), 0, sizeof(struct sockaddr_in));

		nAddrLen = sizeof(struct sockaddr_in);
		hNewSock = accept(stWSAppData.nSock,
			(struct sockaddr FAR*)&stRmtAddr, 
			(int FAR*)&nAddrLen); 
		if (hNewSock == INVALID_SOCKET) {
			nWSAerror = WSAGetLastError();
			if (nWSAerror != WSAEWOULDBLOCK) {
				/* anything but "would block" is bad news */
	    		nb_close (hInst, hwnd);
				WSAperror(nWSAerror, "accept()", hInst);
				wRet = SOCKET_ERROR;
				goto AppExit;
			}
		} else {
			break;	/* connection accepted! */
		}
		/* yield unto the non-operating system */
		OurBlockingHook();
		
	} while (nWSAerror == WSAEWOULDBLOCK);
	/*-------------------------------------*/
	stWSAppData.nSockState = STATE_CONNECTED;
	/*-------------------------------------*/
	
	/* This example is a (very) limited server that can only 
	 *  handle one connection at a time.  So, we close the 
	 * listening socket now and save the newly accepted one */
	wRet = nb_close (hInst, hwnd);	/* close listening socket */
	stWSAppData.nSock = hNewSock;	/* save new (accept) socket */

    /* get timers for statistics updates & I/O (if requested) */                        
	get_timers(hInst, hwnd);
	
AppExit:
	return (wRet);

} /* end as_accept() */

/*---------------------------------------------------------------------------
 * Function: as_dg_cn()
 *
 * Description:
 *  Asynch datagram "connect" for UDP clients
 *
 */
int as_dg_cn (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;		/* Socket structure */
	int	 wRet = SOCKET_ERROR;		/* work variable */

	/* Get a UDP socket */
	stWSAppData.nSock = socket (AF_INET, SOCK_DGRAM, 0);
	if (stWSAppData.nSock == INVALID_SOCKET)  {
        WSAperror(WSAGetLastError(), "socket()", hInst);
        goto AppExit;
	}
	/*---------------------------------*/
	stWSAppData.nSockState = STATE_OPEN;
	/*---------------------------------*/

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
                                               
	/* register for asynchronous notification of all events
	 *  this socket could possibly get.(note: WSAAsyncSelect()
	 *  will also make the socket non-blocking) */
	wRet = WSAAsyncSelect(stWSAppData.nSock, hwnd, IDM_ASYNC,
		(FD_READ | FD_WRITE));
	if (wRet == SOCKET_ERROR) {
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(), "WSAAsyncSelect()", hInst);
	}

	wRet = connect(stWSAppData.nSock,
		(struct sockaddr FAR*)&stLclAddr, 
		sizeof(struct sockaddr_in)); 
	if (wRet == SOCKET_ERROR) {
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(), "connect()", hInst);
		goto AppExit;
	}
      
	/*-------------------------------------*/
	stWSAppData.nSockState = STATE_CONNECTED;
	/*-------------------------------------*/

    /* get timers for statistics updates & I/O (if requested) */                        
	get_timers(hInst, hwnd);

AppExit:    
	return (wRet);

} /* end as_dg_cn() */

/*---------------------------------------------------------------------------
 * Function: as_dg_ls()
 *
 * Description:
 *  Asynch datagram "listen" for TCP servers.
 *
 */
int as_dg_ls (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;		/* Socket structure */
	int	 wRet = SOCKET_ERROR;		/* work variable */

	/* Get a UDP socket */
	stWSAppData.nSock = socket (AF_INET, SOCK_DGRAM, 0);
	if (stWSAppData.nSock == INVALID_SOCKET)  {
		WSAperror(WSAGetLastError(), "socket()", hInst);
        goto AppExit;
	}
	/*---------------------------------*/
	stWSAppData.nSockState = STATE_OPEN;
	/*---------------------------------*/

	/* register for asynchronous notification of all events
	 *  this socket could possibly get.(note: WSAAsyncSelect()
	 *  will also make the socket non-blocking) */
	wRet = WSAAsyncSelect(stWSAppData.nSock, hwnd, IDM_ASYNC,
		(FD_READ | FD_WRITE));
	if (wRet == SOCKET_ERROR) {
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(), "WSAAsyncSelect()", hInst);
	}

	/* convert port number from host byte order to network byte order */
	stLclAddr.sin_port = htons (stWSAppData.nPortNumber);	
	stLclAddr.sin_addr.s_addr = 0L;	/* don't restrict incoming address */
	stLclAddr.sin_family = PF_INET;	/* Internet Address Family */
                                              
	wRet = bind(stWSAppData.nSock,
			(struct sockaddr FAR*)&stLclAddr, 
			sizeof(struct sockaddr_in)); 
	if (wRet == SOCKET_ERROR) {
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(), "bind()", hInst);
		goto AppExit;
	}
	/*---------------------------------*/
	stWSAppData.nSockState = STATE_BOUND;
	/*---------------------------------*/

	do_stats(hwnd, hInst, FALSE);	/* update window to show current state */

AppExit:    
	return (wRet);

} /* end as_dg_ls */


/*---------------------------------------------------------------------------
 * Function: as_w_r()
 *
 * Description:
 *   Asynch write and read.  Our CLIENTS connecting to ECHO port use this.
 */
int as_w_r(HANDLE hInst, HANDLE hwnd)
{
	int	cbOutLen=0;               /* Bytes in Output Buffer */
	int	cbBufSize;				/* Length of I/O */
	int	nLoopLimit;
	int	i,wRet=0,nWSAerror;
	
	cbBufSize = stWSAppData.nLength;
	
	/* sound beep if I/O sound enabled */
	if (stWSAppData.nOptions & OPTION_SOUND)
			MessageBeep(0xFFFF);
			
	/* Adjust the loops per I/O call */
	LoopTimer(TRUE);
		
	/* Read as much as we can first */
	as_r(hInst, hwnd, DFLT_LOOP_MAX);

	nLoopLimit = stWSAppData.nLoopsLeft ? 
		stWSAppData.nLoopsLeft : stWSAppData.nLoopLimit;
	for (i=0; i<nLoopLimit; i++, stWSAppData.wOffset++) {

	    /* adjust offset into output string buffer */
	    if (stWSAppData.wOffset >= cbBufSize) {
	    	stWSAppData.wOffset = 0;
	    }
	     
	    cbOutLen = stWSAppData.nOutLen;	/* pick-up where we left off */
		stWSAppData.nOutLen = 0;		/* reset marker */

	    /* write to server */
	    while (cbOutLen < cbBufSize) {
		    wRet = send (stWSAppData.nSock, 
		    	(LPSTR)&(achChargenBuf[stWSAppData.wOffset+cbOutLen]),
		    	(cbBufSize-cbOutLen), 0);
		    if (wRet == SOCKET_ERROR) {
		    	nWSAerror = WSAGetLastError();
		    	if (nWSAerror != WSAEWOULDBLOCK) {
	    			nb_close (hInst, hwnd);
        			WSAperror(nWSAerror, "send()", hInst);
        		} 

        		/* Keep a marker, so we can pick-up where we leave off */
        		stWSAppData.nOutLen = cbOutLen;
        		stWSAppData.nLoopsLeft = nLoopLimit - i;
        		
			    goto wr_end; /* exit on any error 
			    			 (including WSAEWOULDBLOCK) */
		    } else {
				/* Tally amount sent, to see how much left to send */
		    	cbOutLen += wRet;
		    	stWSAppData.lBytesOut += wRet;
		    	
		    	/* stop if we've reached our limit */
		    	if (stWSAppData.nBytesMax &&
		    		(stWSAppData.lBytesOut >= stWSAppData.nBytesMax)) {
		    			do_close(hInst, hwnd, FALSE);
		    			goto wr_end;
		    	}
		    }     
	    } /* end send() loop */
	} /* end main loop */

	/* Send OOB Data if we want to */
	if (stWSAppData.nOptions & OPTION_OOBSEND) {
		nb_oob_snd (hInst, hwnd);
	}
	
	/* init to normal value (we completed all iterations) */
	stWSAppData.nLoopsLeft = stWSAppData.nLoopLimit;
	
wr_end:
	LoopTimer(FALSE);	/* set exit time from I/O loop */

	return (cbOutLen); 
	
} /* end as_w_r() */

/*---------------------------------------------------------------------------
 * Function: as_r_w()
 *
 * Description:
 *  Asynch read and write.  Our SERVERS providing ECHO service use this.
 */
int as_r_w(HANDLE hInst, HANDLE hwnd)
{
	int		cbInLen;                /* Bytes in Input Buffer */
	int		cbOutLen=0;             /* Bytes in Output Buffer */
	int		cbBufSize;				/* Length of I/O */
	int		i, wRet, nWSAerror;
                                                       
	cbBufSize = stWSAppData.nLength;
	
	if (stWSAppData.nOptions & OPTION_SOUND)
		MessageBeep(0xFFFF);
		
	for (i=0, recv_count=0; i<stWSAppData.nLoopLimit; i++) {
	     
	    /* read as much as we can from client (no less than I/O length
	     *  user requested, but more than that is ok) */
	    cbInLen = 0;
	    while (cbInLen < cbBufSize) {
		    wRet = recv (stWSAppData.nSock, 
			    (LPSTR)&(achInBuf[cbInLen]), BUF_SIZE-cbInLen, 0);
			recv_count++;	/* count recv() calls */
		    if (wRet == SOCKET_ERROR) {
		    	nWSAerror = WSAGetLastError();
		    	if (nWSAerror != WSAEWOULDBLOCK) {
	    			nb_close (hInst, hwnd);
        			WSAperror(nWSAerror, "send()", hInst);
			    	goto rw_exit;
        		}
        		/* exit recv() loop on WSAEWOULDBLOCK */
        		if (cbInLen == 0)
        			goto rw_exit;	/* quit if nothing read yet */
        		else
	        		break;			/* else write out what we read */
        		
		    } else if (wRet == 0) { /* Other side closed socket */
    			MessageBox (hwnd, achOutBuf, 
					"client closed connection", MB_OK);
				do_close (hInst, hwnd, FALSE);
				goto rw_exit;
		    } else {
		    	cbInLen += wRet;
		    	stWSAppData.lBytesIn += wRet;
		    	
		    	/* stop if we've reached our limit */
		    	if (stWSAppData.nBytesMax &&
		    		(stWSAppData.lBytesIn >= stWSAppData.nBytesMax)) {
		    			do_close(hInst, hwnd, FALSE);
		    			break;
		    	}
		    }
        } /* end recv() loop */ 
        
	    /* write to client, as much as we just read */
	    cbOutLen = 0;
	    while (cbOutLen < cbInLen) {
		    wRet = send (stWSAppData.nSock, 
		    	(LPSTR)&(achInBuf[cbOutLen]), 
		    	(cbInLen-cbOutLen), 0);
		    if (wRet == SOCKET_ERROR) {
		    	nWSAerror = WSAGetLastError();
		    	if (nWSAerror != WSAEWOULDBLOCK) {
				    nb_close (hInst, hwnd);
        			WSAperror(nWSAerror, "send()", hInst);
			    	goto rw_end;
        		}
        		break;
		    } else if (wRet == 0) {
		    	/* not likely, but possible */
		    	break;
		    }
			/* Tally amount sent, to see how much left to send */
		    cbOutLen += wRet;
		    stWSAppData.lBytesOut += wRet;
	    } /* end send() loop */
	} /* end main loop */
rw_end:   
	/* Datagram Servers aren't "connected" until data arrives, 
	 *  so change it now (and start timer) */
	if ((stWSAppData.nSockState < STATE_CONNECTED) &&
		(stWSAppData.nSockState > STATE_NONE)) {
	
		/*--------------------------------------*/
	    stWSAppData.nSockState = STATE_CONNECTED;
		/*--------------------------------------*/
		
		/* get timers for statistics updates & I/O (if requested) */
		get_timers(hInst, hwnd);
	}
rw_exit:		
	return (cbOutLen); 
} /* end as_r_w() */

/*---------------------------------------------------------------------------
 * Function: as_r()
 *
 * Description:
 *   Asynch read
 */
int as_r(HANDLE hInst, HANDLE hwnd, int nLoopLimit)
{
	int		cbInLen;
	int		i, wRet, nWSAerror;
                                                       
	if (stWSAppData.nOptions & OPTION_SOUND)
		MessageBeep(0xFFFF);
		
	for (cbInLen=0, recv_count=0, i=0; i<nLoopLimit; i++) {
         
	    /* read as much as we can from server (no less than I/O length
	     *  user requested, but more than that is ok) */
	    wRet = recv (stWSAppData.nSock, 
		    (LPSTR)achInBuf, BUF_SIZE, 0);
		recv_count++;	/* count recv() calls */
	    if (wRet == SOCKET_ERROR) {
	    	nWSAerror = WSAGetLastError();
	    	if (nWSAerror != WSAEWOULDBLOCK) {
			    nb_close (hInst, hwnd);
       			WSAperror(nWSAerror, "send()", hInst);
       		}
		    break;	/* break on any error (including WSAEWOULDBLOCK) */
	    } else if (wRet == 0) { /* Other side closed socket */
    		/* if we're connected, tell user and close */
  			MessageBox (hwnd, achOutBuf, 
				"server closed connection", MB_OK);
			do_close(hInst, hwnd, FALSE);
			break;
	    } else {
		    /* tally amount received */
		    cbInLen += wRet;
			stWSAppData.lBytesIn += wRet;
		    
		    /* stop if we've reached our limit */
		    if (stWSAppData.nBytesMax &&
		    	(stWSAppData.lBytesIn >= stWSAppData.nBytesMax)) {
		    		do_close (hInst, hwnd, FALSE);
		    		break;
		    }
	    }
	} /* end main loop */
	
	/* Datagram Servers aren't "connected" until data arrives, 
	 *  so change it now (and start timer) */
	if ((stWSAppData.nSockState < STATE_CONNECTED) &&
		(stWSAppData.nSockState > STATE_NONE)) {

    	/*--------------------------------------*/
	    stWSAppData.nSockState = STATE_CONNECTED;
		/*--------------------------------------*/
	    
	    get_timers(hwnd, hInst);
	}
	
	return (cbInLen);
} /* end as_r() */

/*---------------------------------------------------------------------------
 * Function: as_w()
 *
 * Description:
 *   Asynch write
 */
int as_w(HANDLE hInst, HANDLE hwnd)
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
		    	if (nWSAerror != WSAEWOULDBLOCK) {
	    			nb_close (hInst, hwnd);
        			WSAperror(nWSAerror, "send()", hInst);
        		}
			    break;	/* break on any error.  If WSAEWOULDBLOCK,
			             *  then FD_WRITE will restart sending */
		    } else {
			    /* Tally amount sent */
			    cbOutLen += wRet;
			    stWSAppData.lBytesOut += wRet;
		    
		    	/* stop if we've reached our limit */
		    	if (stWSAppData.nBytesMax &&
		    		(stWSAppData.lBytesOut >= stWSAppData.nBytesMax)) {
		    			do_close (hInst, hwnd, FALSE);
		    			break;
		    	}
		    }
	    } /* end send() loop */
	} /* end main loop */
	
	/* Send OOB Data if we want to */
	if (stWSAppData.nOptions & OPTION_OOBSEND) {
		nb_oob_snd (hInst, hwnd);
	}

	return (cbOutLen); 
} /* end as_w() */
