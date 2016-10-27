/*---------------------------------------------------------------------
 *
 *  Program: WINSOCKX.LIB  WinSock subroutine library
 *
 *  filename: closecon.c
 * 
 *  copyright by Bob Quinn, 1995
 *   
 *  Description:
 *    This module does a robust graceful close of a TCP connection. 
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
#include <winsock.h>
#include "..\winsockx.h"

/*--------------------------------------------------------------
 * Function: CloseConn()
 *
 * Description: This routine closes a TCP connection gracefully
 *  and robustly:
 *  - first it calls shutdown(how=1) to stop sends but allow 
 *     recieves (this effectively sends a TCP <FIN>).  
 *  - then it calls recv() in a loop to read any remaining data 
 *     into the buffer passed as an input argument.  This makes
 *     sure all data sent by the other side is acknowledged at
 *     the TCP level, and it also allows this end to recieve
 *     all the data without losing any.
 *  - finally, when recv() either fails (with *any* error), or
 *     returns zero (to indicate <FIN> received from other end),
 *     then it calls closesocket() to release the socket.
 */
int CloseConn(SOCKET hSock, LPSTR achInBuf, int len, HWND hWnd)
{
  int nRet=0;
  char achDiscard[BUF_SIZE];
  int cbBytesToDo=len, cbBytesDone=0;
 
  if (hSock != INVALID_SOCKET) {
    /* disable asynchronous notification if window handle provided */
    if (hWnd) {
      nRet = WSAAsyncSelect(hSock, hWnd, 0, 0);
      if (nRet == SOCKET_ERROR)
        WSAperror (WSAGetLastError(), "CloseConn() WSAAsyncSelect()", 0);
    }
  
    /* Half-close the connection to close neatly (NOTE: we ignore the
     *  return from this function because some BSD-based WinSock 
     *  implementations fail shutdown() with WSAEINVAL if a TCP reset 
     *  has been recieved.  In any case, if it fails it means the 
     *  connection is already closed anyway, so it doesn't matter.) */
    nRet = shutdown (hSock, 1);
    
    /* Read remaining data (until EOF or error) */
    for (nRet=1; (nRet && (nRet != SOCKET_ERROR));) {
      if (achInBuf) {
        nRet = recv (hSock,
                     &achInBuf[cbBytesDone],
                     cbBytesToDo, 0);
        if (nRet && (nRet != SOCKET_ERROR)) {
          cbBytesToDo -= nRet;
	  cbBytesDone += nRet;
	}
      } else {
        /* no buffer provided, so discard any data received */
        nRet = recv (hSock, (LPSTR)achDiscard, BUF_SIZE, 0);
      }
      /* close the socket, and ignore any error 
       *  (since we can't do much about them anyway */
      closesocket (hSock);
    }
  }
  return (nRet);
} /* end CloseConn() */
