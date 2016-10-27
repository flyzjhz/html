/*-------------------------------------------------------------------
 *
 *  Program: WAL.EXE  WinSock Application Launcher
 *
 *  filename: wal.h
 *
 *  copyright by Bob Quinn, 1995
 *
 *  Description:
 *   This header file contains all the constant definitions,
 *   macros, structure definitions, external declarations 
 *   and function prototypes used by the modules in the Windows 
 *   Application Launcher (WAL) application.
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
 */
#include "windows.h"
#include "resource.h"
#include <winsock.h>
#include "..\winsockx.h"

#ifdef  BUF_SIZE
#undef  BUF_SIZE
#define BUF_SIZE 4096
#endif

#define WINDOWMENU  1  /* position of window menu     */
#define CBWNDEXTRA  0

#ifdef RC_INVOKED
#define ID(id) id
#else
#define ID(id) MAKEINTRESOURCE(id)
#endif

/* Default WinSock Version (1.1) */
#define WS_VERSION_REQD	0x0101

/* Default Port Numbers */
#define ECHO_PORT      7
#define DISCARD_PORT   9
#define CHARGEN_PORT   19

/* resource ID's */
#define IDWALMENU   ID(1)
#define IDWALWIN    ID(2)
#define IDWALICON   ID(3)

/* Application Option default values */
#define DIX_MSS            1460
#define DFLT_BYTES_PER_IO  DIX_MSS
#define DFLT_LOOP_INTRVAL  100L
#define DFLT_LOOP_MAX      50
#define DFLT_LOOP_UP       5
#define DFLT_LOOP_DN       20		

#define MAX_NAME_SIZE   64

#define STATS_INTERVAL  1000
#define STATS_TIMER_ID  1
#define APP_TIMER_ID    2
#define STATTEXT_HGHT	16
#define X_START_POS     10
#define Y_START_POS     10

#define CONN_RETRIES	6
#define CONN_TIMEOUT	10

#define WS_VERSION_MAJOR 0x01
#define WS_VERSION_MINOR 0x01

/* External functions */
extern LONG CALLBACK WALWndProc (HWND,UINT,WPARAM,LPARAM);
extern BOOL CALLBACK WSAppOptDlgProc (HWND,UINT,WPARAM,LPARAM);
extern BOOL CALLBACK WSIOOptDlgProc (HWND,UINT,WPARAM,LPARAM);
extern BOOL CALLBACK WSIOAdvDlgProc (HWND,UINT,WPARAM,LPARAM);
extern BOOL CALLBACK WSSockOptDlgProc (HWND,UINT,WPARAM,LPARAM);
extern BOOL CALLBACK WSOobOptDlgProc (HWND,UINT,WPARAM,LPARAM);
extern BOOL CALLBACK WSSocketDlgProc (HWND,UINT,WPARAM,LPARAM);
extern void do_stats (HANDLE, HANDLE, BOOL);
extern int do_close (HANDLE, HANDLE, LPARAM);
extern void get_timers (HANDLE, HANDLE);
extern void SetNewTimer (HANDLE, int, int);
extern void LoopTimer (BOOL);
extern int nb_oob_rcv (HANDLE, HANDLE);
extern int nb_oob_snd (HANDLE, HANDLE);
extern BOOL nb_oob_poll (HANDLE, HANDLE);
extern int set_oobinline (HANDLE, HANDLE, BOOL);

/* Debugging functions */
void ShowWinTaskQ(HANDLE);
void ShowSystemQ(HDC, int, int);

/* Blocking functions */
extern int bl_ds_cn (HANDLE, HANDLE);/* blocking datastream connect */
extern int bl_dg_cn (HANDLE, HANDLE);/* blocking datagram "connect" */
extern int bl_ds_ls (HANDLE, HANDLE);/* blocking datastream listen */
extern int bl_dg_ls (HANDLE, HANDLE);/* blocking datagram "listen" */
extern int bl_r_w (HANDLE, HANDLE);	 /* blocking write/read loop */
extern int bl_w_r (HANDLE, HANDLE);	 /* blocking write/read loop */
extern int bl_r (HANDLE, HANDLE);	 /* blocking read loop */
extern int bl_w (HANDLE, HANDLE);	 /* blocking write loop */
extern int bl_close (HANDLE, HANDLE);/* blocking close */

/* Non-Blocking functions */
extern int nb_ds_cn (HANDLE, HANDLE);/* non-blocking datastream connect */
extern int nb_dg_cn (HANDLE, HANDLE);/* non-blocking datagram "connect" */
extern int nb_ds_ls (HANDLE, HANDLE);/* non-blocking datastream listen */
extern int nb_dg_ls (HANDLE, HANDLE);/* non-blocking datagram "listen" */
extern int nb_r_w (HANDLE, HANDLE);	 /* non-blocking write/read loop */
extern int nb_w_r (HANDLE, HANDLE);	 /* non-blocking write/read loop */
extern int nb_r (HANDLE, HANDLE);	 /* non-blocking read loop */
extern int nb_w (HANDLE, HANDLE);	 /* non-blocking write loop */
extern int nb_close (HANDLE, HANDLE);/* non-blocking close */

/* Asynchronous functions (use non-blocking functions for I/O) */
extern int as_ds_cn (HANDLE, HANDLE);/* asynchronous datastream connect */
extern int as_dg_cn (HANDLE, HANDLE);/* asynchronous datagram "connect" */
extern int as_ds_ls (HANDLE, HANDLE);/* asynchronous datastream listen */
extern int as_dg_ls (HANDLE, HANDLE);/* asynchronous datagram "listen" */
extern int as_accept(HANDLE, HANDLE);/* asynchronous write/read loop */
extern int as_r_w (HANDLE, HANDLE);	 /* non-blocking write/read loop */
extern int as_w_r (HANDLE, HANDLE);	 /* non-blocking write/read loop */
extern int as_r (HANDLE, HANDLE, int);/* non-blocking read loop */
extern int as_w (HANDLE, HANDLE);	  /* non-blocking write loop */

extern void OurBlockingHook();	/* our yield function */
void ShowWinTaskQ(HANDLE);	/* to see message queue counts (to debug) */ 

extern int  wChargenLen;	/* length of string to be output */
extern char achChargenBuf[];/* we put CHARGEN_SEQ string resource here */
extern int  cbBufSize;		/* size to send and receive */
                            
extern char achErrBuf [];	/* buffer for error messages */
extern char achInBuf  [];	/* Input Buffer */
extern char achOutBuf [];	/* Output Buffer */

extern int anIoCmd[];		/* table mapping IOMODE index to Message ID */

extern u_long lCallCount;	/* count of calls to I/O function */
extern u_long lLastTicks;	/* GetTickCount() return on last I/O call */

/* Bit Flags to indicate current socket state */
#define STATE_NONE				0x0000
#define STATE_OPEN				0x0001
#define STATE_BOUND				0x0002
#define STATE_LISTENING			0x0004
#define STATE_ACCEPT_PENDING	0x0008
#define STATE_CONN_PENDING		0x0010
#define STATE_CONNECTED			0x0020
#define STATE_CLOSE_PENDING		0x0040

/* Bit Flags for options */
#define OPTION_SOUND			0x0001
#define OPTION_OOBSEND			0x0002
#define OPTION_OOBPOLL			0x0004
#define OPTION_OOBINLINE		0x0008
#define OPTION_OOBOUTSOUND		0x0010
#define OPTION_OOBINSOUND		0x0020
#define OPTION_MASK				0xFFFF

typedef struct wsaapp {
	SOCKET nSock;      /* Socket */
	int  nSockState;   /* Bit Flags to indicate current socket state */
	int	 nRole;    /* Client or Server */
	int	 nProtocol;/* Datastream or Datagram */
	int	 nOpMode;  /* Blocking, Non-Blocking, Asynch, etcetera */
	int  nWinTimer;	/* SetTimer value (if 0, then we're not using a timer) */
	int  nIoMode;   /* I/O Mode (write/read, read/write, read-only or write-only) */
	int  nOptions;	/* Bit flags for options */
	u_short nBytesMax;  /* Total bytes to be sent or received */
	u_long lCallCount;  /* Total number of calls to I/O function */
	u_long lBytesOut;   /* Bytes sent */
	u_long lBytesIn;    /* Bytes received */
	u_long lOobBytesOut;/* Out-of-band bytes sent */
	u_long lOobBytesIn; /* Out-of-band bytes received */
	u_long lStartTime;  /* Interval start time (from GetTickCount()) */
	u_long lSeconds;    /* Current or final time transpired */
	int  wOffset;       /* Offset into Output buffer (chargen sequence) */
	int	 nLoopLimit;/* Limit number of reads and/or writes per I/O */
	int  nLoopMax;      /* Max value LoopLimit can be set to */
	int  nLoopsDn;      /* # Timers same before loop limit decrement */
	int  nLoopsDnMax;
	int  nLoopsUp;      /* # Timers over before loop limit incremented */
	int  nLoopsUpMax;
	int  nLength;       /* Number of bytes for each read and/or write */
	int  nOobOutLen;    /* send() len with MSG_OOB flag */
	int  nOobInLen;     /* recv() len with MSG_OOB flag */
	int  nOobInterval;  /* (non-oob) bytes out interval to send OOB */
	int  nOutLen;	    /* Number of bytes sent if incomplete write */
	int  nLoopsLeft;    /* Number of write loops remaining */
	char szHost[MAX_NAME_SIZE];    /* Hostname or IP Address string */
	char szService[MAX_NAME_SIZE]; /* Service Name */
	u_short nPortNumber;           /* Port Number */
} WSAPP;
extern WSAPP stWSAppData;              /* Application data */

typedef struct opmodeoption {
	char szSection[16];
	int nIoMode;
	int nLength;
	int nLoopLimit;
	int nWinTimer;
	int nBytesMax;
} OPMODEOPTION;

typedef struct iomodeoption {
	int nPortNumber;
	char szService[MAX_NAME_SIZE];
} IOMODEOPTION;

/* Application Types (qualified by: op-mode, protocol, role)
 *
 *      Mode               Protocol         Role
 *      ----               --------         ----
 *      BL: blocking       DS: datastream   CL: client
 *      NB: non-blocking   DG: datagram     SV: server
 *      AS: asynchronous     
 *
 */
#define BL_DS_CL    0
#define NB_DS_CL    1
#define AS_DS_CL    2

#define BL_DG_CL    3
#define NB_DG_CL    4
#define AS_DG_CL    5

#define BL_DS_SV    6
#define NB_DS_SV    7
#define AS_DS_SV    8 

#define BL_DG_SV    9 
#define NB_DG_SV    10
#define AS_DG_SV    11

/* 
 * Application Qualifiers
 */
#define ROLE_CL   0
#define ROLE_SV   1

#define PROT_DS   0
#define PROT_DG   1

#define OPMODE_BL 0
#define OPMODE_NB 1
#define OPMODE_AS 2
#define OPMODE_TH 3

#define IOMODE_WR 0
#define IOMODE_RW 1
#define IOMODE_R  2
#define IOMODE_W  3

