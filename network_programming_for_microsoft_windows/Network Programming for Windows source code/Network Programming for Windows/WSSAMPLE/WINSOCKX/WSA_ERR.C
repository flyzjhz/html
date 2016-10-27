/*---------------------------------------------------------------------
 *
 *  Program: WINSOCKX.LIB  WinSock subroutine library
 *
 *  filename: wsa_err.c
 * 
 *  copyright by Bob Quinn, 1995
 *   
 *  Description:
 *    Library module that retrieves and displays WinSock error text
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

/*---------------------------------------------------------------
 * Function: WSAperror()
 *
 * Description:
 */
void WSAperror (int WSAErr, LPSTR szFuncName, HANDLE hInst) 
{
    static char achErrBuf [ERR_SIZE];	/* buffer for errors */
    static char achErrMsg [ERR_SIZE/2];

    WSAErrStr (WSAErr, (LPSTR)achErrMsg, hInst);
    
	wsprintf (achErrBuf, "%s failed,%-40c\n\n%s",
	  (LPSTR)szFuncName,' ',(LPSTR)achErrMsg);
            
	/* Display error message as is (even if incomplete) */
    MessageBox (GetActiveWindow(), (LPSTR)achErrBuf, (LPSTR)"Error", 
    			MB_OK | MB_ICONHAND);
    return;
}  /* end WSAperror() */

/*-----------------------------------------------------------
 * Function: WSAErrStr()
 *
 * Description: Given a WinSock error value, return error string
 *  NOTE: This function requires an active window to work.
 */
int WSAErrStr (int WSAErr, LPSTR lpErrBuf, HANDLE hInst) {
    int err_len=0;
    HWND hwnd;
    
    if (!hInst) {
      hwnd  = GetActiveWindow();
#ifndef WIN32
      hInst = GetWindowWord(hwnd, GWW_HINSTANCE);
#else
      hInst = (HANDLE) GetWindowLong(hwnd, GWL_HINSTANCE);
#endif
    }
    
    if (WSAErr == 0)           /* If error passed is 0, use the */
      WSAErr = WSABASEERR;     /*  base resource file number */
            
    if (WSAErr >= WSABASEERR)  /* Valid Error code? */
	  /* get error string from the table in the Resource file */
      err_len = LoadString(hInst, WSAErr, lpErrBuf, ERR_SIZE/2);
    
    return (err_len);
}  /* end GetWSAErrStr() */
