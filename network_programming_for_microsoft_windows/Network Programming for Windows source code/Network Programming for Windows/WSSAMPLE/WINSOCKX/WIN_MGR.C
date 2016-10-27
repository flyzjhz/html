/*---------------------------------------------------------------------
 *
 *  Program: WINSOCKX.LIB  WinSock subroutine library
 *
 *  filename: WIN_MGR.C
 *
 *  copyright by Bob Quinn, 1995
 *   
 *  Description:
 *    Library module with common window management functions.
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
#include <windowsx.h>

/*-----------------------------------------------------------
 * Function: CenterWnd()
 *
 * Description: Center window relative to the parent window.  
 */
void CenterWnd(HWND hWnd, HWND hParentWnd, BOOL bPaint) {
  RECT rc2, rc1;
  RECT FAR *lprc;
  int nWidth, nHeight, cxCenter, cyCenter;

  if (!hParentWnd)  /* if we no parent, use desktop! */
    hParentWnd = GetDesktopWindow();
    
  GetWindowRect (hParentWnd, &rc2);
  lprc = (RECT FAR *)&rc2;
  
  cxCenter = lprc->left+((lprc->right-lprc->left)/2);
  cyCenter = lprc->top+((lprc->bottom-lprc->top)/2);

  GetWindowRect (hWnd, &rc1);
  nWidth  = rc1.right-rc1.left;  
  nHeight = rc1.bottom-rc1.top;
	
  MoveWindow (hWnd,
   cxCenter-(nWidth/2),
   cyCenter-(nHeight/2), 
   nWidth, nHeight,
   bPaint);    
  return;	
} /* end CenterWnd() */
