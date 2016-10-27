/*---------------------------------------------------------------------
 *  Program: WAL.EXE  WinSock Application Launcher
 *
 *  filename: oobdata.c
 *
 *  copyright by Bob Quinn, 1995
 *
 *  Description:
 *   This module does all the I/O and other relevant functions for 
 *   Out-of-band (aka "urgent") data on a TCP connection.
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
 ----------------------------------------------------------------------*/
#include <windows.h>
#include <winsock.h> 
#include <string.h>	  /* for _fstrlen() ...could also use memory.h */ 
#include "wal.h"

/*----------------------------------------------------------------------
 *
 * Function: nb_oob_snd()
 *
 * Description:
 *
 */
int nb_oob_snd (HANDLE hInst, HANDLE hwnd)
{
	int wRet, WSAerr;

    hwnd = hwnd;  /* avoid warning (we may need this yet) */
	
	wRet = send (stWSAppData.nSock,
		(LPSTR) &achOutBuf[1],
		stWSAppData.nOobOutLen,
		MSG_OOB);
	if (wRet == SOCKET_ERROR) {
		WSAerr = WSAGetLastError();
		if (WSAerr != WSAEWOULDBLOCK) {
        	WSAperror(WSAerr, "send(MSG_OOB)", hInst);
        }
	}
	
	if (stWSAppData.nOptions & OPTION_OOBOUTSOUND)
		MessageBeep(0xFFFF);
		
	return (wRet);
} /* end nb_oob_snd() */

/*----------------------------------------------------------------------
 *
 * Function: nb_oob_poll()
 *
 * Description:
 *  Polling methods:
 *    - if SO_OOBINLINE disabled (default): select(exceptfds)
 *    - if SO_OOBINLINE enabled ioctlsocket(SIOCATMARK);
 *
 *  (another method for either case possibly recv() 
 *    with both MSG_OOB & MSG_PEEK???  I don't know)
 *
 * Returns:
 *  TRUE if OOB data available somewhere (i.e. socket is in "urgent mode")
 *  FALSE if not
 *
 */
BOOL nb_oob_poll (HANDLE hInst, HANDLE hwnd)
{
	int wRet;
	u_long argp = 0L;
	BOOL bUrgentMode = FALSE;
	fd_set stXcptFDS;
	struct timeval stTimeOut;	/* for select() timeout (none) */
	
	if (stWSAppData.nOptions & OPTION_OOBINLINE) {
	
		/* ioctlsocket(SIOCATMARK) only valid if SO_OOBINLINE is enabled 
		 *  NOTE: SO_OOBINLINE is *disabled* by default */
		wRet = ioctlsocket (stWSAppData.nSock,
			SIOCATMARK,
			(u_long FAR *) argp);
		if (wRet == SOCKET_ERROR) {
        	WSAperror(WSAGetLastError(), "ioctlsocket(SIOCATMARK)", hInst);
		} else {
			/* if argp is set, there's no urgent data; otherwise there is! */
			bUrgentMode = (argp ? FALSE : TRUE);
		}
	} else {
	
		/* select()'s exceptfds indicates urgent mode only if SO_OOBINLINE 
		 *  is disabled.  NOTE: SO_OOBINLINE is disabled by default */

	 	/* clear all sockets from FDS structure, then put our socket 
	  	 *  into the socket descriptor set */
	 	FD_ZERO((fd_set FAR*)&(stXcptFDS));
#pragma message ("-----> WinSock FD_SET macro generates Warning 4127 <-----")	 	
	 	FD_SET(stWSAppData.nSock, (fd_set FAR*)&(stXcptFDS));
	 
	 	/* initialize the timeout structure.  We don't want 
	 	 *  to block at all, so our timeout is zero */
	 	stTimeOut.tv_sec  = 0;
	 	stTimeOut.tv_usec = 0;
	 
	 	wRet = select(-1,				/* call select() */
	 		NULL,
	 		NULL,
	 		(fd_set FAR*)&(stXcptFDS), 
	 		(struct timeval FAR *)&(stTimeOut));
	 		
	 	if (wRet == SOCKET_ERROR) {		/* check return */
	 		/* all errors are bad news */
			WSAperror(WSAGetLastError(), "select(XcptFDS)", hInst);
	 	} 
	 	else if (wRet != 0) {
	 		/* check for OOB Data (exception set) */
	 		if (FD_ISSET (stWSAppData.nSock,
	 			(fd_set FAR*)&(stXcptFDS))) {
	 			
	 			/* We assume that if the exception flag is set, that
	 			 *  it means we have urgent data.  We'd have to call
	 			 *  an I/O function to find the error anyway */
	 			bUrgentMode = TRUE;
	 		} else {
	 			/* This should never happen!!!  If select returned
	 			 *  a positive value, something should be set in
	 			 *  either our exception or our read socket set  */
	 			MessageBox(hwnd,
	 				"Unexpected results from select(XcptFDS)",
	 				"Error", MB_OK | MB_ICONHAND);
	 			nb_close(hwnd, hInst);	/* bail! */
	 		}
	 	}
	}
	return (bUrgentMode);
} /* end nb_oob_poll() */

/*----------------------------------------------------------------------
 *
 * Function: nb_oob_rcv()
 *
 * Description:
 *   This really should read all the normal data into buffer, 
 *    *then* read the urgent data.
 *
 */
int nb_oob_rcv (HANDLE hInst, HANDLE hwnd)
{
	int wRet, WSAerr;
	
	wRet = recv (stWSAppData.nSock,
		(LPSTR) achInBuf,
		stWSAppData.nLength,
		MSG_OOB);
	if (wRet == SOCKET_ERROR) {
		WSAerr = WSAGetLastError();
		if (WSAerr != WSAEWOULDBLOCK) {
        	WSAperror(WSAerr, "recv(MSG_OOB)", hInst);
        }
	}
	
	if (stWSAppData.nOptions & OPTION_OOBINSOUND)
		MessageBeep(0xFFFF);
		
	if (stWSAppData.nIoMode == IOMODE_RW) {
		/* Echo Oob with Oob, if that's what we're doing */
		nb_oob_snd (hInst, hwnd);		
	}
		
	return (wRet);
} /* end nb_oob_rcv() */

/*----------------------------------------------------------------------
 *
 * Function: set_oobinline()
 *
 * Description:
 *   Enables or disables the SO_OOBINLINE option according to whether
 *   the boolean flag passed is set or not.
 *
 */
int set_oobinline(HANDLE hInst, HANDLE hwnd, BOOL bOobInLine)
{
	int wRet;

    hwnd = hwnd;  /* avoid warning (we may need this yet) */

	wRet = setsockopt (stWSAppData.nSock,
    	SOL_SOCKET,
    	SO_OOBINLINE,
    	(const char FAR *)&bOobInLine,
    	sizeof (BOOL));
    if (wRet == SOCKET_ERROR) {
       	WSAperror(WSAGetLastError(), "setsockopt(SO_OOBINLINE)", hInst);
    }
	return (wRet);    
} /* end set_oobinline() */

 