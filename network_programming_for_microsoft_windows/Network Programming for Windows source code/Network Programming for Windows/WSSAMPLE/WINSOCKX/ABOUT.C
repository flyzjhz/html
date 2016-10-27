/*---------------------------------------------------------------------
 *
 *  Program: WINSOCKX.LIB  WinSock subroutine library
 *
 *  filename: about.c
 *  
 *  copyright by Bob Quinn, 1995
 *   
 *  Description:
 *    This module has the common dialog routine for the about
 *    box used by all sample applications for _Windows Sockets
 *    Network Programming_ by Bob Quinn and Dave Shute.    
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

/*---------------------------------------------------------------------
 *
 * Function: Dlg_About()
 *
 * Description:
 */
BOOL CALLBACK Dlg_About (
	HWND hDlg,
	UINT msg,
	UINT wParam,
	LPARAM lParam)
{
    BOOL bRet = FALSE;
    char achDataBuf[WSADESCRIPTION_LEN+1];
    
    lParam = lParam;	/* avoid warning */

    switch (msg) {
        case WM_INITDIALOG:
 	        wsprintf (achDataBuf, "(Compiled: %s, %s)\n", 
  		        (LPSTR)__DATE__, (LPSTR)__TIME__);
            SetDlgItemText (hDlg, IDC_COMPILEDATE, (LPCSTR)achDataBuf);
            wsprintf (achDataBuf, 
            	"Version: %d.%d", 
            	LOBYTE(stWSAData.wVersion),  /* major version */
            	HIBYTE(stWSAData.wVersion)); /* minor version */
            SetDlgItemText (hDlg, IDS_DLLVER, (LPCSTR)achDataBuf);
            wsprintf (achDataBuf, 
            	"HiVersion: %d.%d", 
            	LOBYTE(stWSAData.wVersion),  /* major version */
            	HIBYTE(stWSAData.wVersion)); /* minor version */
            SetDlgItemText(hDlg,IDS_DLLHIVER, achDataBuf);            
            SetDlgItemText(hDlg,IDS_DESCRIP,(LPCSTR)(stWSAData.szDescription));
            SetDlgItemText(hDlg,IDS_STATUS,(LPCSTR)(stWSAData.szSystemStatus));
            wsprintf (achDataBuf,"MaxSockets: %u", stWSAData.iMaxSockets);
            SetDlgItemText (hDlg, IDS_MAXSOCKS, (LPCSTR)achDataBuf);
            wsprintf (achDataBuf,"iMaxUdp: %u", stWSAData.iMaxUdpDg);
            SetDlgItemText (hDlg, IDS_MAXUDP, (LPCSTR)achDataBuf);
            CenterWnd (hDlg, GetActiveWindow(), TRUE);
            break;
	    
	    case WM_COMMAND:
    	    switch (wParam) {
        	    case IDOK:
            	    EndDialog (hDlg, 0);
            	    bRet = TRUE;
            }
        break;
    }
    return (bRet);
} /* end Dlg_About() */

