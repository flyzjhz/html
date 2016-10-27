/*--------------------------------------------------------------------------- 
 *  Program: WAL.EXE  WinSock Application Launcher
 *
 *  filename: nonblkng.c, 
 *
 *  copyright by Bob Quinn, 1995
 *
 *  Description:
 *   This module contains the Windows Sockets network code in a
 *   sample application that does non-blocking calls.
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
 ---------------------------------------------------------------------------*/
#include <windows.h>
#include <windowsx.h>
#include <winsock.h> /* Windows Sockets */
#include <string.h>  /* for _fxxxxx() functions */ 

#include "resource.h"
#include "..\winsockx.h"
#include "wal.h"

/*---------------------------------------------------------------------------
 * Function: nb_ds_cn()
 *
 * Description:
 *  Blocking datastream connect.
 *
 */
int nb_ds_cn (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;	/* Socket structure */
	int	wRet = SOCKET_ERROR, i;		/* work variables */
	fd_set stWritFDS;	/* select() "file descriptor set" structures */
	fd_set stXcptFDS;
	struct timeval stTimeOut;	/* for select() timeout (none) */
	BOOL bInProgress = FALSE;
	u_long	lOnOff;
	int nWSAerror;

	/* Get a TCP socket (if we don't have one already) */
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
	stLclAddr.sin_addr.s_addr = GetAddr (stWSAppData.szHost);  

	/* make our socket non-blocking */
	lOnOff = TRUE;
	wRet = ioctlsocket(stWSAppData.nSock, FIONBIO, (u_long FAR *)&lOnOff);
	if (wRet) {	/* ioctlsocket() returns 0 on success */
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(), (LPSTR)"ioctlsocket()", hInst);
		goto AppExit;
	}

	/* convert port number from host byte order to network byte order */
	stLclAddr.sin_port = htons (stWSAppData.nPortNumber);
	stLclAddr.sin_family = PF_INET;	/* Internet Address Family */
    
    /*----------------------------------------*/                                        
     stWSAppData.nSockState = STATE_CONN_PENDING;
    /*----------------------------------------*/                                        

#if 0
/*------------------- Example of what NOT to do ---------------------------
 * This is one algorithm for detecting a TCP connection on a non-blocking
 *  socket that was used in BSD Sockets.  Don't do this!
 *
 * We don't connect() non-blocking this way because it is too dependent
 *  on specific error values to detect socket state.  It does not work
 *  because on Microsoft TCP/IP and others the second call to connect() 
 *  call gives WSAEINVAL instead of the expected (BSD-compatible) 
 *  WSAEALREADY.
 *  
 * The more robust and reliable way to detect the connection on a non-
 *  blocking socket--other than WSAAsyncSelect()--is to use select() 
 *  (which blocks).
 */
	i = 0;
	do {
		i++;
		wRet = connect(stWSAppData.nSock,
			(struct sockaddr FAR*)&stLclAddr,
			sizeof(struct sockaddr_in));

		/* on non-blocking connect, we expect errors:
		 *   - WSAEISCONN means success
		 *   - some mean retry
		 *   - some others are bad news
		 */
		if (wRet == SOCKET_ERROR) {
			nWSAerror = WSAGetLastError();
			if (nWSAerror == WSAEISCONN) {
				/* we're connected!! */
				break;
			
			} else if  ((nWSAerror == WSAEWOULDBLOCK) ||
						(nWSAerror == WSAEINPROGRESS) ||
						(nWSAerror == WSAEALREADY)) {
				/* need to retry */
				bInProgress = TRUE;
			} else {
				/* fatal error */
				wsprintf (achOutBuf, "connect(%d)", i);
	    		nb_close (hInst, hwnd);
				WSAperror(nWSAerror, achOutBuf, hInst);
				goto AppExit;
			}
		}		
	} while (bInProgress);
/*---------------------End of example what NOT to do---------------*/
#endif

	/*-------------------- begin using select() --------------------
	 * The proper (BSD-compatible) way to do a non-blocking connect()
	 *  in a loop does not work on many WinSocks (which return WSAEINVAL
	 *  on 2nd and subsequent connect() calls, instead of WSAEALREADY).
	 *
	 * Although using select() to detect connection completion is a
	 *  blocking call, it is the best method on a non-blocking socket.
	 *
	 * We'll allow a certain number of timeouts (e.g. select() returns
	 *  the value 0), then fail.
	 */
	 /* Now wait for "writability" to indicate connection completion, 
	  *  or an exception to indicate an error */
	 for (i= 0;;) {		/* we deal with counter inside the loop */
		wRet = connect(stWSAppData.nSock,
			(struct sockaddr FAR*)&stLclAddr,
			sizeof(struct sockaddr_in));

		/* on non-blocking connect, we expect WSAEWOULDBLOCK error 
		 *  but WSAEALREADY is ok too, and WSAEISCONN means we're 
		 *  done! 
		 */
		if (wRet == SOCKET_ERROR) {
			nWSAerror = WSAGetLastError();
			if (nWSAerror == WSAEISCONN) {
				/* we're connected!! */
				break;
			} else if  ((nWSAerror != WSAEWOULDBLOCK) &&
						(nWSAerror != WSAEALREADY)) {
				/* fatal error */
				wsprintf (achOutBuf, "connect(%d)", i);
	     		nb_close (hInst, hwnd);
				WSAperror(nWSAerror, achOutBuf, hInst);
				goto AppExit;
			}
		}		
		
 	    /* clear all sockets from FDS structure, then put our socket 
  	     *  into the socket descriptor set */
	    FD_ZERO((fd_set FAR*)&(stWritFDS));
	    FD_ZERO((fd_set FAR*)&(stXcptFDS));
#pragma	message ("--> WinSock FD_SET macro generates MSVC warning C4127 <--")
	    FD_SET(stWSAppData.nSock, (fd_set FAR*)&(stWritFDS));
	    FD_SET(stWSAppData.nSock, (fd_set FAR*)&(stXcptFDS));
	 
	 	/* initialize the timeout structure.  We use the same timeout as
	  	 *  our I/O polling loop.  However, we don't want to poll with
	  	 *  a zero timeout, since we don't have a blocking hook function
	  	 *  to yield we'll depend on WinSock's default blocking hook.
	  	 *  We use a nice round number for our timeout: 1 second. */
	 	stTimeOut.tv_sec  = CONN_TIMEOUT;
	 	stTimeOut.tv_usec = 0;
	 
	 	wRet = select(-1,				/* call select() */
	 		NULL,
	 		(fd_set FAR*)&(stWritFDS), 
	 		(fd_set FAR*)&(stXcptFDS), 
	 		(struct timeval FAR *)&(stTimeOut));
	 		
	 	if (wRet == SOCKET_ERROR) {		/* check return */
	 		/* all errors are bad news */
	    	nb_close (hInst, hwnd);
			WSAperror(WSAGetLastError(), "select()", hInst);
	 		goto AppExit;
	 		
	 	} else if (wRet > 0) {
	 		/* check for error (exception) first */
	 		if (FD_ISSET (stWSAppData.nSock,
	 			(fd_set FAR*)&(stXcptFDS))) {
	 			/* all errors are bad news */
	    		nb_close (hInst, hwnd);
				WSAperror(WSAGetLastError(),"select(excptn)", hInst);
	 			goto AppExit;
	 		}

	 		if (FD_ISSET (stWSAppData.nSock,(fd_set FAR*)&(stWritFDS))) {
	 			/* If "writable" socket set includes ours, we're connected! */
	 			break;
	 		} else {
	 			/* This should never happen!!!  If select returned
	 			 *  a positive value, something should be set in
	 			 *  either our exception or our read socket set  */
	 			MessageBox(hwnd,
	 				"Unexpected results from select()",
	 				"Error", MB_OK | MB_ICONHAND);
	 			nb_close(hwnd, hInst);
	 			goto AppExit;	/* bail! */
	 		} 
	 	} else {	/* wRet == 0 which indicates select() timeout */
	 		i++;	/* increment our timeout counter */
	 		if (i++ >= CONN_RETRIES) {
	 			/* if we've had our fill of retries, report error and exit */
	 			MessageBox(hwnd,
	 				"select() timeout exceeded",
	 				"Error", MB_OK | MB_ICONHAND);
	 			nb_close(hwnd, hInst);
	 			goto AppExit;
	 		}
	 	}
	 }
	/*-------------------- end using select() --------------------*/

	/*-------------------------------------*/
	stWSAppData.nSockState = STATE_CONNECTED;
	/*-------------------------------------*/
                            
    /* get timers for statistics updates & I/O (if requested) */                        
	get_timers(hInst, hwnd);
      
	/* Tell ourselves to do next step! */
	PostMessage(hwnd, WM_COMMAND, anIoCmd[stWSAppData.nIoMode], 0L);

AppExit:    
	return (wRet);

} /* end nb_ds_cn() */

/*---------------------------------------------------------------------------
 * Function: nb_ds_ls()
 *
 * Description:
 *  Blocking datastream listen/accept
 *
 */
int nb_ds_ls (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;	/* Socket structures */
	struct sockaddr_in stRmtAddr;
	int	 nAddrLen;		/* for length of address structure */	
	u_long lOnOff;		/* on/off switch for ioctlsocket() command */
	SOCKET hNewSock;	/* new socket returned from accept() */
	fd_set stReadFDS;	/* select() "file descriptor set" structures */
	fd_set stXcptFDS;
	struct timeval stTimeOut;	/* for select() timeout (none) */
	int	wRet = SOCKET_ERROR;	/* work variables */
	int nWSAerror;

	/* Get a TCP socket (if we don't already have one) */
	if (stWSAppData.nSockState == STATE_NONE) {
		stWSAppData.nSock = socket (AF_INET, SOCK_STREAM, 0);
		if (stWSAppData.nSock == INVALID_SOCKET)  {
			WSAperror(WSAGetLastError(),"socket()", hInst);
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

	/* make our socket non-blocking */
	lOnOff = TRUE;
	wRet = ioctlsocket(stWSAppData.nSock, FIONBIO, (u_long FAR *)&lOnOff);
	if (wRet) {	/* ioctlsocket() returns 0 on success */
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(), "ioctlsocket()", hInst);
		goto AppExit;
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
		WSAperror(WSAGetLastError(),"bind()", hInst);
		goto AppExit;
	}
	/*---------------------------------*/
	stWSAppData.nSockState = STATE_BOUND;
	/*---------------------------------*/

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
	
	/*-------------------- begin using select() --------------------
	 * There is no good way to detect an incoming connection with
	 *  a non-blocking socket (or non-blocking operation).  Looping
	 *  on accept() is possible (expecting WSAEWOULDBLOCK), but it
	 *  incurs lots of CPU overhead. 
	 *
	 *  Since a non-blocking loop requires a blocking hook function 
	 *  anyway (to yield in Windows 3.x), we might as well use a 
	 *  blocking operation.  Using the select() function to detect
	 *  "readability" is the most common and acceptable way to do 
	 *  detect an incoming connection request (i.e. the "time to
	 *  accept()" event).
	 */
	 /* Now wait for "readability", or an error (exception) */
	 do {
	 	/* clear all sockets from FDS structure, then put our socket 
	  	*  into the socket descriptor set */
	 	FD_ZERO((fd_set FAR*)&(stReadFDS));
	 	FD_ZERO((fd_set FAR*)&(stXcptFDS));
#pragma	message ("--> WinSock FD_SET macro generates MSVC warning C4127 <--")	
	 	FD_SET(stWSAppData.nSock, (fd_set FAR*)&(stReadFDS));
	 	FD_SET(stWSAppData.nSock, (fd_set FAR*)&(stXcptFDS));
	 
	 	/* initialize the timeout structure.  We use the same timeout as
	  	 *  our I/O polling loop.  However, we don't want to poll with
	  	 *  a zero timeout, since we don't have a blocking hook function
	  	 *  to yield we'll depend on WinSock's default blocking hook.
	  	 *  We use a nice round number for our timeout: 1 second. */
	 	stTimeOut.tv_sec  = 1;
	 	stTimeOut.tv_usec = 0;
	 
	 	wRet = select(-1,				/* call select() */
	 		(fd_set FAR*)&(stReadFDS), 
	 		NULL,
	 		(fd_set FAR*)&(stXcptFDS), 
	 		(struct timeval FAR *)&(stTimeOut));
	 		
	 	if (wRet == SOCKET_ERROR) {		/* check return */
	 		/* all errors are bad news */
	    	nb_close (hInst, hwnd);
			WSAperror(WSAGetLastError(), "select()", hInst);
	 		goto AppExit;
	 		
	 	} else if (wRet != 0) {
	 		/* check for error (exception) first */
	 		if (FD_ISSET (stWSAppData.nSock,
	 			(fd_set FAR*)&(stXcptFDS))) {
	 			/* all errors are bad news */
			    nb_close (hInst, hwnd);
				WSAperror(WSAGetLastError(), "select(excptn)", hInst);
	 			goto AppExit;
	 		}
	 		if (!(FD_ISSET (stWSAppData.nSock,
	 			 (fd_set FAR*)&(stReadFDS)))) {
	 			/* This should never happen!!!  If select returned
	 			 *  a positive value, something should be set in
	 			 *  either our exception or our read socket set  */
	 			MessageBox(hwnd,
	 				"Unexpected results from select()",
	 				"Error", MB_OK | MB_ICONHAND);
	 			nb_close(hwnd, hInst);
	 			goto AppExit;	/* bail! */
	 		}
	 	}
	 } while (wRet == 0);	/* 0 return means select() timeout expired */
	/*-------------------- end using select() --------------------*/
  
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
                                    
	/* Tell ourselves to do next step! */
	PostMessage(hwnd, WM_COMMAND, anIoCmd[stWSAppData.nIoMode], 0L);

AppExit:    
	return (wRet);

} /* end nb_ds_ls() */

/*---------------------------------------------------------------------------
 * Function: nb_dg_cn()
 *
 * Description:
 *  Blocking datagram "connect".
 *
 */
int nb_dg_cn (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;		/* Socket structure */
	int	 wRet = SOCKET_ERROR;		/* work variable */
	u_long lOnOff;

	/* Get a UDP socket */
	if (stWSAppData.nSock) {
		stWSAppData.nSock = socket (AF_INET, SOCK_DGRAM, 0);
		if (stWSAppData.nSock == INVALID_SOCKET)  {
        	WSAperror(WSAGetLastError(), "socket()", hInst);
        	goto AppExit;
        }
	}
	/*---------------------------------*/
	stWSAppData.nSockState = STATE_OPEN;
	/*---------------------------------*/

	/* make our socket non-blocking */
	lOnOff = TRUE;
	wRet = ioctlsocket(stWSAppData.nSock, FIONBIO, (u_long FAR *)&lOnOff);
	if (wRet) {	/* ioctlsocket() returns 0 on success */
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(), "ioctlsocket()", hInst);
		goto AppExit;
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
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(),"connect()", hInst);
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

} /* end nb_dg_cn() */

/*---------------------------------------------------------------------------
 * Function: nb_dg_ls()
 *
 * Description:
 *  Blocking datagram "listen".
 *
 */
int nb_dg_ls (HANDLE hInst, HANDLE hwnd)
{
	struct sockaddr_in stLclAddr;		/* Socket structure */
	int	 wRet = SOCKET_ERROR;		/* work variable */
	u_long lOnOff;

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

	/* make our socket non-blocking */
	lOnOff = TRUE;
	wRet = ioctlsocket(stWSAppData.nSock, FIONBIO, (u_long FAR *)&lOnOff);
	if (wRet) {	/* ioctlsocket() returns 0 on success */
	    nb_close (hInst, hwnd);
		WSAperror(WSAGetLastError(), "ioctlsocket()", hInst);
		goto AppExit;
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

	/* Tell ourselves to do next step! */
	PostMessage(hwnd, WM_COMMAND, anIoCmd[stWSAppData.nIoMode], 0L);
	
AppExit:    
	return (wRet);

} /* end nb_dg_ls */

/*---------------------------------------------------------------------------
 * Function: nb_w_r()
 *
 * Description:
 *   Non-blocking write and read.  Our CLIENTS connecting to ECHO port 
 *   (by default) use this.
 */
int nb_w_r(HANDLE hInst, HANDLE hwnd)
{
	int		cbOutLen=0;             /* Bytes in Output Buffer */
	int		cbBufSize;				/* Length of I/O */
	int		i,wRet,nWSAerror;
	static  int bBusyFlag = FALSE;	/* semaphore */
	
	if (bBusyFlag)	/* use a semaphore to avoid being reentered */
		return(0);	/*  when yielding (the nesting can be fatal) */
	else
		bBusyFlag = TRUE;
                                                       
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
        			WSAperror(nWSAerror,"send()", hInst);
			    	goto wr_end;
        		}
		    } else {
			/* Tally amount sent, to see how much left to send */
		    	cbOutLen += wRet;
		    	stWSAppData.lBytesOut += wRet;
		    }
	    } /* end send() loop */

	    /* read as much as we can from the server, until we get an 
	     *  error, any error (most likely one is WSAEWOULDBLOCK
	     *  which in effect means there's no more data to be read) */
	    for (;;) {
		    wRet = recv (stWSAppData.nSock, (LPSTR)achInBuf, BUF_SIZE, 0);
		    if (wRet == SOCKET_ERROR) {
		    	nWSAerror = WSAGetLastError();
		    	if (nWSAerror != WSAEWOULDBLOCK) {
				    nb_close (hInst, hwnd);
        			WSAperror(nWSAerror, "recv()", hInst);
			    	goto wr_end;	/* quit on error */
        		}
        		break;				/* WouldBlock is not fatal */
		    } else if (wRet == 0) { /* Other side closed socket */
      			MessageBox (hwnd, achOutBuf, 
					"server closed connection", MB_OK);
				do_close (hInst, hwnd, FALSE);
				goto wr_end;
		    } else {
		    	stWSAppData.lBytesIn += wRet;
		    }
        } /* end recv() loop */

       	OurBlockingHook();	/* yield */
        
	} /* end main loop */
   
wr_end:
	            
	/* Tell ourselves to do it again (if we're not 
	 *  using a timer and we're still connected)! */
	if (!stWSAppData.nWinTimer &&
	   (stWSAppData.nSockState == STATE_CONNECTED))
		PostMessage(hwnd, WM_COMMAND, IDM_WRITE_READ, 0L);

	bBusyFlag = FALSE;	/* reset semaphore */

	return (cbOutLen); 
} /* end nb_w_r() */

/*---------------------------------------------------------------------------
 * Function: nb_r_w()
 *
 * Description:
 *   Blocking read and write.  Our SERVERS providing ECHO service use this.
 */
int nb_r_w(HANDLE hInst, HANDLE hwnd)
{
	int		cbInLen;                /* Bytes in Input Buffer */
	int		cbOutLen=0;             /* Bytes in Output Buffer */
	int		cbBufSize;				/* Length of I/O */
	int		i, wRet, nWSAerror;
	static  int bBusyFlag = FALSE;	/* semaphore */
	
	if (bBusyFlag)	/* use a semaphore to avoid being reentered */
		return(0);	/*  when yielding (the nesting can be fatal) */
	else
		bBusyFlag = TRUE;
                                                       
	cbBufSize = stWSAppData.nLength;	/* get the I/O size */

	if (stWSAppData.nOptions & OPTION_SOUND)	                                                       
		MessageBeep(0xFFFF);
		
	for (i=0; i<stWSAppData.nLoopLimit; i++, stWSAppData.wOffset++) {
	     
	    /* read as much as we can from client (no less than I/O length
	     *  user requested, but more than that is ok) */
	    cbInLen = 0;
	    while (cbInLen < cbBufSize) {
		    wRet = recv (stWSAppData.nSock, 
			    (LPSTR)achInBuf, BUF_SIZE, 0);
		    if (wRet == SOCKET_ERROR) {
		    	nWSAerror = WSAGetLastError();
		    	if (nWSAerror != WSAEWOULDBLOCK) {
				    nb_close (hInst, hwnd);
        			WSAperror(nWSAerror,"send()", hInst);
			    	goto rw_end;
        		}
        		/* exit recv() loop on WSAEWOULDBLOCK */
        		if (cbInLen == 0)
        			goto rw_end;	/* quit if nothing read yet */
        		else
	        		break;			/* else write out what we read */
        		
		    } else if (wRet == 0) { /* Other side closed socket */
       			MessageBox (hwnd, achOutBuf, 
					"client closed connection", MB_OK);
				do_close (hInst, hwnd, FALSE);
				goto rw_end;
		    } else {
		    	cbInLen += wRet;
		    	stWSAppData.lBytesIn += wRet;
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
        		
		    } 
			/* Tally amount sent, to see how much left to send */
		    cbOutLen += wRet;
		    stWSAppData.lBytesOut += wRet;
	    } /* end send() loop */
        
		OurBlockingHook();	/* yield */

	} /* end main loop */
   
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
		
rw_end:
	            
	/* Tell ourselves to do it again (if we're not 
	 *  using a timer and we're still connected)! */
	if (!stWSAppData.nWinTimer && 
	    (stWSAppData.nSockState == STATE_CONNECTED)) {
		PostMessage(hwnd, WM_COMMAND, IDM_READ_WRITE, 0L);
	}
                        
	bBusyFlag = FALSE;	/* reset semaphore */
                        
	return (cbOutLen); 
} /* end nb_r_w() */

/*---------------------------------------------------------------------------
 * Function: nb_r()
 *
 * Description:
 *   Non-blocking read
 */
int nb_r(HANDLE hInst, HANDLE hwnd)
{
	int		cbInLen;                /* Bytes in Input Buffer */
	int		cbBufSize;				/* Length of I/O */
	char	achInBuf  [BUF_SIZE];   /* Input Buffer */
	int		i, wRet, nWSAerror;
	static  int bBusyFlag = FALSE;	/* semaphore */
	
	if (bBusyFlag)	/* use a semaphore to avoid being reentered */
		return(0);	/*  when yielding (the nesting can be fatal) */
	else
		bBusyFlag = TRUE;
                                                       
	cbBufSize = stWSAppData.nLength;

	if (stWSAppData.nOptions & OPTION_SOUND)	                                                       
		MessageBeep(0xFFFF);
		
	for (cbInLen=0, i=0; i<stWSAppData.nLoopLimit; i++) {

	    /* read as much as we can from server */
	    wRet = recv (stWSAppData.nSock, 
		    (LPSTR)achInBuf, BUF_SIZE, 0);
	    if (wRet == SOCKET_ERROR) {
	    	nWSAerror = WSAGetLastError();
	    	if (nWSAerror != WSAEWOULDBLOCK) {
			    nb_close (hInst, hwnd);
       			WSAperror(nWSAerror, "send()", hInst);
       		}
		    break;	/* break on any error (including WSAEWOULDBLOCK) */
		    
	    } else if (wRet == 0) { /* Other side closed socket */
   			MessageBox (hwnd, achOutBuf, 
				"server closed connection", MB_OK);
			do_close(hInst, hwnd, FALSE);
			break;
			
	    } else {
		    /* tally amount received */
		    cbInLen += wRet;
		    
			/* Datagram Servers aren't "connected" until 
	 		 *  data arrives,  so change it now (and start timer) */
			if ((stWSAppData.nSockState < STATE_CONNECTED) &&
				(stWSAppData.nSockState > STATE_NONE)) {
			
				/*--------------------------------------*/
				stWSAppData.nSockState = STATE_CONNECTED;
				/*--------------------------------------*/
	    
				get_timers(hwnd, hInst);
			}
	    }
	    
	    OurBlockingHook();	/* yield */
	    
	} /* end main loop */
	
	/* tally what we read */
	stWSAppData.lBytesIn += cbInLen;
	
	/* Tell ourselves to do it again (if we're not 
	 *  using a timer and we're still connected)! */
//	if ((!stWSAppData.nWinTimer) && 
//	   	(stWSAppData.nSockState == STATE_CONNECTED))
		PostMessage(hwnd, WM_COMMAND, IDM_READ, 0L);

	bBusyFlag = FALSE;	/* reset semaphore */

	return (cbInLen);
} /* end nb_r() */

/*---------------------------------------------------------------------------
 * Function: nb_w()
 *
 * Description:
 *   Non-blocking write
 */
int nb_w(HANDLE hInst, HANDLE hwnd)
{
	int		cbOutLen=0;             /* Bytes in Output Buffer */
	int		cbBufSize;				/* Length of I/O */
	int		i, wRet, nWSAerror;
	static  int bBusyFlag = FALSE;	/* semaphore */
	
	if (bBusyFlag)	/* use a semaphore to avoid being reentered */
		return(0);	/*  when yielding (the nesting can be fatal) */
	else
		bBusyFlag = TRUE;
                                                       
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
        			WSAperror(nWSAerror,"send()", hInst);
        		}
        		break; /* break on any error */
		    } else {
			    /* Tally amount sent */
			    cbOutLen += wRet;
			    stWSAppData.lBytesOut += wRet;
		    }
	    } /* end send() loop */
	    
		OurBlockingHook();	/* yield */
		
	} /* end main loop */

	/* Tell ourselves to do it again (if we're not 
	 *  using a timer and we're still connected)! */
	if (!stWSAppData.nWinTimer && 
	   (stWSAppData.nSockState == STATE_CONNECTED))
		PostMessage(hwnd, WM_COMMAND, IDM_WRITE, 0L);
		
	bBusyFlag = FALSE;	/* reset semaphore */

	return (cbOutLen); 
} /* end nb_w() */

/*---------------------------------------------------------------------------
 * Function: nb_close()
 *
 * Description:
 *  Differences between this and a non-blocking close is that this cannot 
 *  handle an WSAEWOULDBLOCK error gracefully (i.e. ignore it ), and this  
 *  will* handle WSAEINPROGRESS gracefully.
 */
int nb_close (HANDLE hInst, HANDLE hwnd) 
{   
	int wRet, nWSAerror;	/* work variables */
	
	/* we may have already set this state earlier, 
	 *  but not in all cases */
	/*------------------------------------------*/
	stWSAppData.nSockState = STATE_CLOSE_PENDING;	
	/*-----------------------------------------*/

	/* Cancel any pending blocking operation */
	if (WSAIsBlocking()) {
		WSACancelBlockingCall();
	}
	                                       
    /* Doing a close without calling shutdown(how=1), and looping on
     *  recv() first--the way we do in CloseConn() in our WinSockX.lib--
     *  is dangerous and not robust ...but we're experimenting here */
	nWSAerror = 0;	                                                     
    wRet = closesocket(stWSAppData.nSock);
    if (wRet == SOCKET_ERROR) {
    	nWSAerror = WSAGetLastError();
		/* if not "would block" error, report it! */
    	if ((nWSAerror != WSAEWOULDBLOCK) && IsWindow(hwnd)) {
       		WSAperror(nWSAerror, "closesocket()", hInst);
       	}
	}
    
    /* Socket is invalid now (if it wasn't already) */
	/*---------------------------------*/
    stWSAppData.nSockState = STATE_NONE;
	/*---------------------------------*/
	
	/* free the timer resource(s) (if window not destroyed yet) */
	if (IsWindow (hwnd)) {
		KillTimer (hwnd, STATS_TIMER_ID);
		if (stWSAppData.nWinTimer)
			KillTimer (hwnd, APP_TIMER_ID);
	}

	return (wRet);
}  /* end nb_close() */

/*---------------------------------------------------------------------------
 * Function: OurBlockingHook()
 *
 * Description:
 */
void OurBlockingHook () {
	BOOL bRet;
	MSG msg;
	int i;
	                         
	/* loop on messages, but don't loop forever */
	for (i = 0; i < DFLT_LOOP_MAX; i++) {
		bRet = (BOOL)PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (bRet) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			break;	/* leave, if nothing to do */
		}
	}
	return;
} /* end OurBlockingHook() */

