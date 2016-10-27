/*---------------------------------------------------------------------
 *
 *  Program: WINSOCKX.LIB  WinSock subroutine library
 *
 *  filename: wsa_addr.c
 *
 *  copyright by Bob Quinn, 1995
 *   
 *  Description:
 *    Library module for common host address procedures.
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
#include "..\winsockx.h"

#ifdef WIN32
#define _fstrcmp strcmp
#endif
#include <string.h>
/*-----------------------------------------------------------
 * Function: GetHostID()
 *
 * Description: 
 *  Get the Local IP address using the following algorithm:
 *    - get local hostname with gethostname()
 *    - attempt to resolve local hostname with gethostbyname()
 *    if that fails:
 *    - get a UDP socket
 *    - connect UDP socket to arbitrary address and port
 *    - use getsockname() to get local address
 */
LONG GetHostID () {
    char szLclHost [MAXHOSTNAME];
    LPHOSTENT lpstHostent;
    SOCKADDR_IN stLclAddr;
    SOCKADDR_IN stRmtAddr;
    int nAddrSize = sizeof(SOCKADDR);
    SOCKET hSock;
    int nRet;
    
    /* Init local address (to zero) */
    stLclAddr.sin_addr.s_addr = INADDR_ANY;
    
    /* Get the local hostname */
    nRet = gethostname(szLclHost, MAXHOSTNAME); 
    if (nRet != SOCKET_ERROR) {
      /* Resolve hostname for local address */
      lpstHostent = gethostbyname((LPSTR)szLclHost);
      if (lpstHostent)
        stLclAddr.sin_addr.s_addr = *((u_long FAR*) (lpstHostent->h_addr));
    } 
    
    /* If still not resolved, then try second strategy */
    if (stLclAddr.sin_addr.s_addr == INADDR_ANY) {
      /* Get a UDP socket */
      hSock = socket(AF_INET, SOCK_DGRAM, 0);
      if (hSock != INVALID_SOCKET)  {
        /* Connect to arbitrary port and address (NOT loopback) */
        stRmtAddr.sin_family = AF_INET;
        stRmtAddr.sin_port   = htons(IPPORT_ECHO);
        stRmtAddr.sin_addr.s_addr = inet_addr("128.127.50.1");
        nRet = connect(hSock,
                       (LPSOCKADDR)&stRmtAddr,
                       sizeof(SOCKADDR));
        if (nRet != SOCKET_ERROR) {
          /* Get local address */
          getsockname(hSock, 
                      (LPSOCKADDR)&stLclAddr, 
                      (int FAR*)&nAddrSize);
        }
        closesocket(hSock);   /* we're done with the socket */
      }
    }
    return (stLclAddr.sin_addr.s_addr);
} /* GetHostID() */


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

