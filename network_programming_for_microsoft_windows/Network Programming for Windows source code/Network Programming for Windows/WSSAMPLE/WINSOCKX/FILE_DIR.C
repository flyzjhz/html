/*---------------------------------------------------------------------
 *
 *  Program: WINSOCKX.LIB  WinSock subroutine Library
 *
 *  filename: file_dir.c
 *
 *  copyright by Bob Quinn, 1995
 *   
 *  Description: These are non-network related routines:
 *    GetLclDir(): retrieves the local directory of files
 *    CreateLclFile(): creates a file
 *    File_Dlg(): prompt user for filename (or directory)
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
#include <dos.h>
#ifdef _WIN32
#include <io.h>        /* for Microsoft 32bit find file structure */
#else
#include <direct.h>    /* for Microsoft 16bit find file structure */
#endif
#include <string.h>    /* for strcpy() */

#include "..\winsockx.h"

/*---------------------------------------------------------------
 * Function: GetLclDir()
 *
 * Description: Get the local file directory and write to
 *   temporary file for later display.
 */
BOOL GetLclDir(LPSTR szTempFile)
{
#ifdef WIN32
  struct _finddata_t stFile;  /* Microsoft's 32bit find file structure */
#else
  struct _find_t stFile;      /* Microsoft's 16bit find file structure */
#endif
  HFILE hTempFile;
  int nNext;

  hTempFile = CreateLclFile (szTempFile);

  if (hTempFile != HFILE_ERROR) {
#ifdef WIN32
    nNext =_findfirst("*.*", &stFile);
    while (!nNext)  {
      wsprintf(achTempBuf, " %-12s %.24s  %9ld\n",
               stFile.name, ctime( &( stFile.time_write ) ), stFile.size );
      _lwrite(hTempFile, achTempBuf, strlen(achTempBuf));
      nNext = _findnext(nNext, &stFile);
    }
#else
    nNext = _dos_findfirst("*.*",0,&stFile);
    while (!nNext)  {
      unsigned month, day, year, hour, second, minute;
      month  =  (stFile.wr_date >>5)   & 0xF;
      day    =   stFile.wr_date & 0x1F;
      year   = ((stFile.wr_date >> 9)  & 0x7F) + 80;
      hour   =  (stFile.wr_time >> 11) & 0x1F;
      minute =  (stFile.wr_time >> 5)  & 0x3F;
      second =  (stFile.wr_time & 0x1F) << 1;
      wsprintf(achTempBuf,          
        "%s\t\t%ld bytes \t%d-%d-%d \t%.2d:%.2d:%.2d\r\n",
        stFile.name, stFile.size, month, day, year, hour, minute, second);
      _lwrite(hTempFile, achTempBuf, strlen(achTempBuf));
      nNext = _dos_findnext(&stFile);
    }
#endif
    _lclose (hTempFile);
    return (TRUE);
  }
  return (FALSE);  
} /* end GetLclDir() */

/*---------------------------------------------------------------
 * Function: CreateLclFile()
 *
 * Description: Try to create a file on local system, and if it
 *  fails notify user and prompt for new local filename;
 */
HFILE CreateLclFile (LPSTR szFileName) {
  HFILE hFile;
  char szRmtFile[MAXFILENAME];

  hFile = _lcreat (szFileName, 0);  /* create the file */
  
  strcpy (szRmtFile, szFileName);   /* save remote filename */
  while (hFile == HFILE_ERROR) {
    wsprintf(achTempBuf, 
      "Unable to create file %s.  Change the name.", szFileName);
    MessageBox (hWinMain, achTempBuf,
      "File Error", MB_OK | MB_ICONASTERISK);
    if (!DialogBox (hInst, MAKEINTRESOURCE(IDD_FILENAME), 
      hWinMain, Dlg_File)) {
      /* No new filename provided, so quit */  
      break;
    } else {
      /* Try to create new filename */
      hFile = _lcreat (szFileName, 0);
    }
  }
  strcpy (szFileName, szRmtFile);  /* replace remote filename */
  
  return (hFile);
} /* end CreateLclFile() */  

/*---------------------------------------------------------------------
 * Function: Dlg_File()
 *
 * Description:  Prompt for a file name (also used for directory names)
 */                                        
BOOL CALLBACK Dlg_File (
  HWND hDlg,
  UINT msg,
  UINT wParam,
  LPARAM lParam)  /* This param must be set with DialogBoxParam() call 
	           *  to pass a pointer to the filename buffer */
{
   BOOL bRet = FALSE;
   static LPSTR szDataFile;
   
   lParam = lParam;  /* avoid warning */

   switch (msg) {
     case WM_INITDIALOG:
       CenterWnd (hDlg, hWinMain, TRUE);
       szDataFile = (LPSTR) lParam;       
       break;
     case WM_COMMAND:
       switch (wParam) {
         case IDOK:
           GetDlgItemText (hDlg, IDC_FILE, szDataFile, MAXFILENAME);
           EndDialog (hDlg, TRUE);
           bRet = TRUE;
           break;
         case IDCANCEL:
           EndDialog (hDlg, FALSE);
           bRet = FALSE;
           break;
       }
   }        
   return(bRet);
} /* end Dlg_File() */
