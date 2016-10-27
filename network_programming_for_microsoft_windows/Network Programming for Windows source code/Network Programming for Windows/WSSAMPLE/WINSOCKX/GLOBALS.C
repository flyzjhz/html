/*---------------------------------------------------------------------
 *
 *  Program: WINSOCKX.LIB  WinSock subroutine library
 *
 *  filename: globals.c
 *
 *  copyright by Bob Quinn, 1995
 *   
 *  Description:
 *    Globals used by winsockx library and common to its users
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
#include "..\winsockx.h"
#include "..\wsa_xtra.h"

WSADATA stWSAData;              /* WinSock DLL Info */

char *aszWSAEvent[] = {         /* For error messages */
  "unknown FD_ event",
  "FD_READ",
  "FD_WRITE",
  "FD_OOB",
  "FD_ACCEPT",
  "FD_CONNECT",
  "FD_CLOSE"
};

char achTempBuf [BUF_SIZE]={0};  /* Screen I/O data buffer and such */
char szTempFile []="delete.me";  /* Temporary work file */

HWND hWinMain;                   /* Main window (dialog) handle */
HINSTANCE hInst;                 /* Instance handle */

int nAddrSize = sizeof(SOCKADDR);