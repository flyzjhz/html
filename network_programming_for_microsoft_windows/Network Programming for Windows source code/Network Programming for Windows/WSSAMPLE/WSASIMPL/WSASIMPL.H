/*---------------------------------------------------------------------
 *
 * Program: WSASIMPL.DLL  Simplified WinSock API for TCP Clients)
 *
 * filename: wsasimpl.h
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
#include <winsock.h>
#ifndef IDC_STATIC
#define IDC_STATIC -1
#endif
extern SOCKET WINAPI ConnectTCP(LPSTR, LPSTR);
extern int WINAPI SendData(SOCKET, LPSTR, int);
extern int WINAPI RecvData(SOCKET, LPSTR, int, int);
extern int WINAPI CloseTCP(SOCKET, LPSTR, int);

