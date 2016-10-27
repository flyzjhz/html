/*------------------------------------------------------------------------
 *  Program: WAL.EXE  WinSock Application Launcher
 *
 *  filename: debug.c
 *
 *  Description: Contains routines for debugging WAL.  Need to link
 *   with TOOLHELP.LIB (or add code to load it), to use ShowSystemQ()
 *   Notice that ShowSystemQ() does nothing in Win32 (otherwise it
 *   would cause a GPF).
 *
 *  NOTE: Most of the code in this module is derived from code in
 *   the text _Undocumented Windows_ by Andrew Schulman, David
 *   Maxey & Matt Pietrek, (c) 1992 by Andrew Schulman & David
 *   Maxey, published by Addison-Wesley Publishing Company
 *   ISBN 0-201-60834-0
 --------------------------------------------------------------------------*/
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h> 
#include <windowsx.h>

#include "wal.h" 

/*----------------- Debugging stuff (from TOOLHELP.H) ---------------------*/
typedef struct {	/* Windows 3.x Task Queue (aka Message Queue) */
	WORD wNext;	/*  as described p385 _Undocumented Windows_ */
	HANDLE hTask;	/*  used below to display task's message queue */
	WORD msgSize;
	WORD msgCount;
	WORD nextMessageOffset;
	WORD nextFreeMessageOffset;
	WORD endofQueue;
	DWORD GetMessageTimeRetval;
	DWORD GetMessagePosRetval;
	WORD messageQueueStart;
} TASKQ;
typedef struct tagGLOBALENTRY {
    DWORD dwSize;
    DWORD dwAddress;
    DWORD dwBlockSize;
    HGLOBAL hBlock;
    WORD wcLock;
    WORD wcPageLock;
    WORD wFlags;
    BOOL wHeapPresent;
    HGLOBAL hOwner;
    WORD wType;
    WORD wData;
    DWORD dwNext;
    DWORD dwNextAlt;
} GLOBALENTRY;
#define GLOBAL_ALL      0
#define GT_DATA         2
#ifndef WIN32
/* These are from toolhelp.DLL */
BOOL    WINAPI GlobalFirst(GLOBALENTRY FAR* lpGlobal, WORD wFlags);
BOOL    WINAPI GlobalNext(GLOBALENTRY FAR* lpGlobal, WORD wFlags);
#endif

#define MK_FP(s,o) ((void far *) (((DWORD) (s) << 16) | (o)))
void ShowSystemQ (HDC, int, int);

/* These two are from MSVC\INCLUDE\DOS.H */
#define FP_SEG(fp) (*((unsigned FAR *)&(fp)+1))
#define FP_OFF(fp) (*((unsigned FAR *)&(fp)))

#include <shellapi.h>		// TEST

/*------------------------------------------------------------------------
 * Function: ShowWinTaskQ()
 *
 * Description: display summary of current Task's Windows Message Queue 
 *   status for the SockPointer passed as parameter.
 */
void ShowWinTaskQ(HANDLE hwnd)
{
	char MsgBuf[80];
	HINSTANCE hKernel;
	WORD q_size;
	MSG far *firstMsg;
	WORD (FAR PASCAL *lpfnGetTaskQueue) (HANDLE);
	HANDLE hTaskQ;
	TASKQ far *fpTaskQ;
	
	/* Get handle for the Windows Kernel and address for GetTaskQueue() */
	hKernel = (HINSTANCE) GetModuleHandle("KERNEL");
	(FARPROC) lpfnGetTaskQueue =
		GetProcAddress(hKernel, "GETTASKQUEUE");
	
	/* Get the handle to the Task Queue (aka Message Queue for Task) */
	hTaskQ = lpfnGetTaskQueue(GetWindowTask(hwnd));
	
	/* Display the Task ID (associated socket) and Message Count (which
	 *  may be same as task's Message Queue capacity since we likely
	 *  just entered this routine after failing to post a message to
	 *  a valid window).
	 */
	if (hTaskQ) {
		fpTaskQ  = MK_FP(hTaskQ, 0);	/* ptr to TaskQ struct */
		firstMsg = MK_FP(hTaskQ, 0x6e); /* offset assumes Win3.1 */
		
		/* calculate the total size of the message queue available */
		q_size = (WORD) ((fpTaskQ->endofQueue - FP_OFF(firstMsg)) / 
			 (fpTaskQ->msgSize));
			
		wsprintf(MsgBuf,"msgCount:%d, q-size:%d\n", 
		     fpTaskQ->msgCount, q_size);
		OutputDebugString(MsgBuf);
	}
	return;
} /* end ShowWinTaskQ() */
    
/*------------------------------------------------------------------
 * Function: ShowSystemQ()
 *
 * Description:  Looks at the current system message queue.  Code
 *  is from page 501 in _Undocumented Windows_
 *-----------------------------------------------------------------*/
void ShowSystemQ(HDC hdc, int XPos, int YPos)
{
#ifndef WIN32
	static BYTE far *fpMsgs;
	static TASKQ far *SysMsgQ;
	static WORD hSysMsgQ;
	GLOBALENTRY ge;
	WORD wVers, QOfs, cMsg;
	BOOL ok;
	WORD hUser = GetModuleHandle("USER");
	WORD UserOtherDS = 0;
	WORD numpending, nextmsg;

	// Use ToolHelp to find USER's non-default data segment
	ge.dwSize = sizeof(ge);
	ok = GlobalFirst (&ge, GLOBAL_ALL);
	while (ok) {
		if (ge.hOwner == hUser) {
			if (ge.wType == GT_DATA) {  /* not GT_DGROUP! */
				UserOtherDS = ge.hBlock;
				break;
			}
		}
		ok = GlobalNext(&ge, GLOBAL_ALL);
	}
	if (UserOtherDS == 0)
		return;	/* couldn't locate USER's non-DGROUP DS */
		
	/* Get selector to System Message Queue */
	wVers = (WORD) GetVersion();
	QOfs = 0;	/* assuming Windows 3.1 */
	hSysMsgQ = *((WORD FAR*) MK_FP(UserOtherDS, QOfs));
	
	SysMsgQ = MK_FP(hSysMsgQ, 0);
	
	/* figure out number of messages in System Message Queue */
	cMsg = GetProfileInt("windows", "TypeAhead", 120);
	
	/* figure out offset of actual queue in Sys Msg Q struct */
	QOfs = SysMsgQ->endofQueue - (cMsg * SysMsgQ->msgSize);
	fpMsgs = MK_FP(hSysMsgQ, QOfs);
	
	/* show where SysMsgQ is, and how big it is */
	wsprintf(achOutBuf, "SysMsgQ: %u %u-byte messages",
		cMsg,
		SysMsgQ->msgSize);
	TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
	YPos += STATTEXT_HGHT;
	
	/* show how many unretrieved messages are in the queue */
	nextmsg = SysMsgQ->nextMessageOffset;
	numpending = (SysMsgQ->nextFreeMessageOffset - nextmsg) /
		SysMsgQ->msgSize;
	wsprintf(achOutBuf, "pending system msgs: %d", numpending);
	TextOut(hdc, XPos, YPos, achOutBuf, _fstrlen((LPSTR)achOutBuf));
#endif		
} /* end ShowSystemQ() */
