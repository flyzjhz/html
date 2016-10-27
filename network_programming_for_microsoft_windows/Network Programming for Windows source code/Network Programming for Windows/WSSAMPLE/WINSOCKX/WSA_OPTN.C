/*---------------------------------------------------------------------
 *
 *  Program: WINSOCKX.LIB  WinSock subroutine library
 *
 *  filename: wsa_optn.C
 *
 *  copyright by Bob Quinn, 1995
 *   
 *  Description:
 *    Library module for common sockets options functions.
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
#include "..\wsa_xtra.h" 
#include <windows.h>
#include <windowsx.h>
#include "..\winsockx.h"

#include <winsock.h>

/*-----------------------------------------------------------
 * Function: GetBuf()
 *
 * Description: Center window relative to the parent window.  
 */
int GetBuf (SOCKET hSock, int nBigBufSize, int nOptval) {
  int nRet, nTrySize, nFinalSize=0;
                     
  for (nTrySize=nBigBufSize; 
    nTrySize>MTU_SIZE; 
    nTrySize>>=1) {
    nRet = setsockopt(hSock, SOL_SOCKET, nOptval, 
      (char FAR*)&nTrySize, sizeof(int));
    if (nRet == SOCKET_ERROR) {
      int WSAErr = WSAGetLastError();
      if ((WSAErr==WSAENOPROTOOPT) || (WSAErr==WSAEINVAL))
        break;
    } else {
      nRet = sizeof(int);
      getsockopt(hSock, SOL_SOCKET, nOptval, 
        (char FAR *)&nFinalSize, &nRet);
      break;
    }
  }
  return (nFinalSize); 
} /* end GetBuf() */
