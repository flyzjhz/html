---------------------------------------------------------------------
 _Windows Sockets Network Programming_  by Bob Quinn and Dave Shute,
   published by Addison-Wesley, Reading, MA  ISBN: 0-201-63372-8
---------------------------------------------------------------------

Welcome!  This is the "readme" file for the Windows Sockets samples 
disk that accompanies _Windows Sockets Network Programming_.  This 
file contains:

  - Installation Notes
  - Names of installed directories and their contents
  - Descriptions of each of the six sample WinSock applications
  - Description of the sample DLL over WinSock
  - Description of the sample WinSock subroutine library

This is not the only documentation on this disk.  The index.htm 
file in the webpage directory has the contents of this file and a 
lot of other useful information in hypertext (HTML) format.  You 
can use any Internet browser application like Mosaic or Netscape 
to access this hypertext documentation.

Note: Everything on this disk and more are also available from our
web page on the Internet at http://www.sockets.com  Please email 
your comments, suggestions and bug resports to webmaster@sockets.com  

--------------------------------------------------------------------
Installation Notes:
-------------------

 - If you use this installation program to install the samples on
   Windows NT, the program aborts when it attempts to place the
   icons in the group box.  This is after all the files have been
   copied, so you can use the Program Manager File/New/Program Item
   command to add the icons manually.  Most of them run the 16 and
   32-bit applications installed in the "BIN" directory.  The icons 
   are as follows (replace the directory "C:\WSA" with the path you
   used during installation of the samples):

    Description  Command Line
    -----------  ------------
    ac_ftp       C:\WSA\BIN\AC_FTP.EXE
    as_echo      C:\WSA\BIN\AS_ECHO.EXE
    multitst     C:\WSA\BIN\MULTITST.EXE
    simpltst     C:\WSA\BIN\SIMPLTST.EXE
    ua_time      C:\WSA\BIN\UA_TIME.EXE
    wal          C:\WSA\BIN\WAL
    ac_ftp32     C:\WSA\BIN\AC_FTP32.EXE
    asecho32     C:\WSA\BIN\ASECHO32.EXE
    multi_32     C:\WSA\BIN\MULTI_32.EXE
    simplt32     C:\WSA\BIN\SIMPLT32.EXE
    uatime32     C:\WSA\BIN\UATIME32.EXE
    wal32        C:\WSA\BIN\WAL32
    readme       notepad C:\WSA\BIN\ECHO.LOG
    echo.log     notepad C:\WSA\BIN\ECHO.LOG
    ftp.log      notepad C:\WSA\BIN\FTP.LOG
    WinSock 1.1  winhelp C:\WSA\WINSOCK.HLP
    
  - If you are running 16-bit Windows (versions 3.0, 3.1 or 3.11),
    and you do NOT have Win32s--the 32-bit emulator--installed,
    then you cannot run the 32-bit versions of each application
    (and Windows will identify each as a non-Windows MS-DOS appli-
    cation, and assign an MS-DOS icon accordingly).
    
  - If you run setup.exe more than once, it will create duplicate
    icons for each entry in the WinSock Samples program group.

 --------------------------------------------------------------------
Contents of Each Directory
--------------------------

Assuming you installed the Windows Sockets samples in a directory
named "WSA", the subdirectories and their contents are as follows:

 WSA          - Windows Sockets development files (.H, .LIB, etc.)
 WSA\AC_FTP   - source code for a ftp client application (TCP)
 WSA\AS_ECHO  - source code for a echo server application (TCP)
 WSA\BIN      - 16 and 32-bit executables for all samples
 WSA\MULTITST - source code for UDP multicast and ICMP ping
 WSA\UA_TIME  - source code for a daytime client and server (UDP)
 WSA\WAL      - a flexible WinSock tester application (UDP & TCP)
 WSA\WEB      - hypertext documentation in HTML format
 WSA\WINSOCKX - sample WinSock subroutines used by samples
 WSA\WSASIMPL - source code for a DLL over WinSock that provides 
                a simplified WinSock API, and source code for a 
                simple echo client application that uses it.

---------------------------------------------------------------------
Sample Applications
-------------------

All but one of the sample applications available here are used
as examples in the text _Windows Sockets Network Programming_.

  - AC_FTP.EXE: FTP client application (asynchronous operation 
     mode with a TCP socket)
  - AS_ECHO.EXE: echo server application (asynchronous operation 
     mode with a TCP socket)
  - UA_TIME.EXE: daytime client and server application (asynchronous 
      operation mode with a UDP socket)
  - MULTITST.EXE: optional features test application (asynchronous 
     multicast UDP and SOCK_RAW ICMP ping)
  - WAL.EXE: flexible WinSock implementation test, exploration, and 
     benchmark tool (not pretty, but does everything)

We used Microsoft Visual C++ enviroments (versions 1.52 for 16-bit,
and version 2.1 for 32-bit) to create most of the makefiles.
Unfortunately, because the paths are hard-coded in the file, you will
have to bring the project files (.mak) into the respective MS C++
environments to readjust things to the new directory, and even then
you will have to manually alter the project to access the library
files (the are in the root of the directory where you install the
samples).

All samples--including the sample DLL and static library--have a
number of other things in common:

  - They all have 32-bit versions, and all 32-bit version names
     end with "32" (16-bit versions don't have a number).
  - They use the WSAperror() function from #WINSOCKX">WINSOCKX.LIB
    to display error values and short descriptions when an unexpected
    error occurs.
  - They display error messages on any suspicious error condition.
     They don't hide errors, but report them whenever they occur. As
     we describe n a_c.htm">Appendix C: WinSock Error Reference,
     these error messages should appear only when a "user fixable
     error" occurs. If you get an error message from a sample
     application for a non user-fixable error, then this may
     indicate an anomoly in the WinSock implementation that your applications
     may need to deal with. We show you the errors on purpose, to make
     you aware of unexpected conditions.
  - They have a minimal user interface, and do not have help (.HLP)
     files available.
  - They are meant to be played with. They are for exploration
     and experimentation as well as to illustrate how to write WinSock
     applications.

The icons used for each sample don't mean much, but they meet
the following three criteria:

    - They each contain the socket from the official WinSock icon.
    - Each one is colorful in its own unique and wonderful way.
    - Each took under 10 minutes to create.

------------------------------------------------------------------------
AC_FTP.EXE: file transfer protocol client application 
-----------------------------------------------------

This application is described in detail in Chapter 7, "Sample
Application and Library."

What it does: AC_FTP is a file transfer protocol client
 that uses asynchronous socket operation mode and utilizes many
 programming techniques described in detail throughout the book.
 It connects to an FTP server, sends commands and receives replies
 according to rfc959.txt">RFC 959 to get and put files,
 change directories, get file lists, delete files.

What it illustrates: The source code for AC_FTP illustrates
 many key aspects of WinSock application programming, including
 how to connect as a TCP client and as a server, how to maximize
 data throughput using sockets with asynchronous operation mode.
 It also demonstrates how to receive variable length data on a
 stream connection; this is often a problem since TCP (rfc793.txt">RFC 793
 and rfc1122.txt">RFC 1122) does not preserve packet
 boundaries. It also shows how to implement an application protocol--the
 FTP protocol--using a state machine; maintaining socket state
 is an important aspect of most network applications.

How to Use it: AC_FTP implements a subset of the FTP command
 set, and has buttons in the main dialog box for each of the commands
 available..

  - Start by pressing the "Connect" button, and fill
     in the destination hostname or IP address (the location of the
     FTP server), the user name and password.. Once the "Status"
     in upper right of the AC_FTP application window says "connected,"
     you can press the other buttons to issue FTP commands
  - The "DIR" button in the "Remote Working Directory"
     box.retrieves the remote file directory, and "DIR" in
     the "Local Working Directory" displays the file directory
     from the local disk. AC_FTP puts the file directory list into
     a temporary file called delete.me, and invokes the Windows notepad.exe
     to display it. You can leave several such directory file lists
     open for simultaneos viewing
  - The "CWD" buttons allow you move to different directories
     on the remote or local systems.
  - The "Put" and "Get" buttons send and receive
     files, respectively.
  - You can "Abort" any pending operation, "Close" the control 
     connection, or "Exit" the application at any time.

Known Problems: AC_FTP, as all other the sample applications,
 suffers from a user-hostile interface. User friendliness was sacrificed
 to minimize user interface code, and emphasize the network code.
 The most blatant user-hostile feature is the text in the status
 box that scrolls in the wrong direction. The method of displaying
 the file directories is less than ideal also, but calls for very
 little code. The CWD command does not allow you to move between
 disks on the local system.

File List: 
 AC_FTP\AC_FTP.DEF 
 AC_FTP\AC_FTP.ICO 
 AC_FTP\RESOURCE.H 
 AC_FTP\AC_FTP.H 
 AC_FTP\AC_FTP.MAK Makefile for 16-bit ac_ftp.exe 
 AC_FTP\AC_FTP32.MAK Makefile for 32.bit ac_ftp32.exe 
 AC_FTP\AC_FTP.RC 
 AC_FTP\FTP_DATA.C all routines for the data connection 
 AC_FTP\FTP_CTRL.C all routines for the control connection 
 AC_FTP\AC_FTP.C

------------------------------------------------------------------------
AS_ECHO.EXE: echo server application
------------------------------------

This application is described in Chapter 6, "Socket States."

What it does: AS_ECHO provides the standard TCP echo service
 (on port 7), as described by rfc862.txt">RFC 862. The application 
 reads data from any number of echo clients simultaneously, and sends 
 the data received from each client back to the client that sent it.

What it illustrates: AS_ECHO illustrates how to write a
 fast and efficient TCP server capable of handling multiple connections
 simultaneously using asynchronous operation mode.

How to Use it: As is the case with most server applications,
 AS_ECHO has a minimal user interface; it is mainly driven by clent
 connections. After you run it, it displays the local host name
 and IP address (if it could be retrieved using our GetHostID()
 routine from WINSOCKX.LIB). The main dialog box displays the number 
 of active connections, the number of connections  that have 
 disconnected, the total number of bytes received is
 updated realtime, and the data rate of the last connection is
 displayed as each client disconnects. The data rate and address
 of each connection are also logged to a file named as_echo.log.
 The application does not have any options, and the only interaction
 possible is with either the "About" button to display
 the about box, or the "Exit" button to quit the program.

Known Problems: If AS_ECHO is running on a WinSock implementation
 that uses the same buffer pool for input and output, and a client
 sends data faster than it reads data, then AS_ECHO will stop working.
 In this case it cannot send because all system buffers are full
 of incoming data, and it has no application buffer space available
 to read the data from the system buffers.

 For example, this is a problem running WAL asynch echo client
 against AS_ECHO. To avoid the problem, you can adjust the I/O
 options in the WAL client so the number of loops and loop limit
 are 1. This will lower the data throughput considerably, but you
 can increase that by decreasing the timeout to 50 milliseconds.

File List:
 AS_ECHO\AS_ECHO.DEF 
 AS_ECHO\AS_ECHO.H 
 AS_ECHO\AS_ECHO.ICO 
 AS_ECHO\RESOURCE.H 
 AS_ECHO\AS_ECHO.MAK Makefile for 16-bit as_echo.exe 
 AS_ECHO\ASECHO32.MAK Makefile for 32-bit asecho32.exe 
 AS_ECHO\AS_ECHO.RC 
 AS_ECHO\AS_ECHO.C

------------------------------------------------------------------------
MULTITST.EXE: multicast and ping test application
-------------------------------------------------

This application is referenced in Chapter 16, "Optional Features."


What it does: The MULTITST application allows sending and receiving of 
 multicast datagrams RFC 1112 and RFC 1122), and experimentation with
 the multicast APIs. MULTITST also sends and receives Internet
 Control Message Protocol RFC 791 "echo request" and "echo reply" 
 datagrams, typically known as "ping" packets. and uses the IP_TTL 
 socket option to set the IP time-to-live field for traceroute.

What it illustrates: It shows how to use the multicast,
 and SOCK_RAW sockets for ICMP (with IP_TTL).

How to Use it: The two parts to this program--multicast
 UDP and ICMP Ping--have seperate interfaces. We'll describe the
 steps involved with using each seperately. To play with multicast,
 you need to get a socket by selecting the "Socket" menu
 item. Once you have a socket, you can send to multicast addresses,
 or any other IP address with the Sendto menu item at any point.
 However, receiving multicast packets requires additional steps.

 The steps required to receive multicast datagrams, sometimes differ
 between different multicast implementations. For Microsoft's,
 for example, you must call call bind() (with the Bind menu item)
 next. For bind() you can select any class D address, and any port
 number (or use 0 to let the TCP/IP stack assign an arbitrary port).
 Next step involves socket options, which you can access with the
 "SockOpts" menu selection. The most important option
 is IP_ADD_MEMBERSHIP, which joins the multicast group to allow
 you to receive datagrams destined for that address. You should
 specify the same class D address for the multicast group that
 you used in the call to bind(). In some cases, you may also have
 to use the IP_MULTICAST_IF option to specify the default interface.
 Here is a short description of these and other multicast-related
 options; for more information, see Chapter 16: "Optional
 Features"

  - IP_ADD_MULTICAST: join multicast group to receive multicast
     datagrams
  - IP_DROP_MULTICAST: leave a multicast group
  - IP_MULTICAST_LOOP: disable or enable multicast loopback (enabled
     by default)
  - IP_MULTICAST_TTL: set the IP "Time to Live" for
     multicast (it is only one by default, which means multicast don't
     go beyond the first router)
  - IP_MULTICAST_IF: set the multicast interface

*Important Note* The "BSD option values" is set by default
 to use the BSD-compatible values for the option name macro values.
 You will need to disable this setting for multicast implementations
 --like Microsoft's--that use the Steve Deering values for each
 multicast option.

 To send and receive ICMP "ping" datagrams, select the
 "Ping" menu item, and the ping interface will be displayed.
 First, press the "socket()" button to get raw socket
 to use for ICMP. Next, enter an IP address or hostname as the
 "Destination host." At this point you can press "sendto()"
 to send an ICMP echo request packet, and MULTITST will display
 a message box when the response arrives. You could change the
 ICMP ID and sequence numbers if you wish; these change the values
 of the ICMP fields as described in RFC791.  You can also try 
 changing the time to live value in the IP header RFC 791), 
 although very few WinSock implementations support the BSD-compatible 
 IP_TTL socket option that the MULTITST application uses.

Known Problems: MULTITST notifies the user of each multicast
 datagram and ping echo response arrival by displaying a message 
 box. This is less than ideal, especially when receiving ping responses
 sent to broadcast addresses, or when using the traceroute option
 (by setting IP TTL).

File List: 
 MULTITST\MULTITST.DEF 
 MULTITST\MULTITST.ICO 
 MULTITST\ICMPPING.H 
 MULTITST\RESOURCE.H 
 MULTITST\MULTITST.MAK Makefile for 16-bit multitst.exe 
 MULTITST\MULTI_32.MAK Makefile for 32-bit multi_32.exe 
 MULTITST\MULTITST.H 
 MULTITST\ICMPPING.C 
 MULTITST\MULTITST.RC 
 MULTITST\MULTITST.C

------------------------------------------------------------------------
UA_TIME.EXE: daytime client and server application
--------------------------------------------------

This application is referenced in Chapter 5, "Operation Modes."

What it does: UA_TIME works as a client and a server using
 UDP in asynchronous operation mode to implement the simple daytime
 protocol, as described in RFC 867. As a client, it sends a datagram 
 to the daytime service (port 13), then reads the response, which 
 contains a human-readable string indicating the current date and 
 time maintained by the daytime server system. As a server, it 
 responds to each unsolicited datagram received on port 13, but 
 returning a human-readable date/time string.

What it illustrates: It shows how to create a combination
 client and server that uses a UDP socket RFC 768 and asynchronous 
 operation mode. It also allows uses setsockopt() SO_BROADCAST 
 to allow sending and recieving broadcast datagrams.

How to Use it: The program starts without a socket, so
 the first thing you need to do is use the "Open" command
 to get a socket, and call bind(). You then use "Sendto"
 to send requests to daytime server systems. If you use "Options"
 to enable the use of broadcast destination addresses, you can
 send to the limited broadcast address (255.255.255.255) to query
 all the hosts on your local network. Each time a response is received,
 UA_TIME displays the time/date string along with the source address
 in a message box.

Known Problems: This application should display each response
 in a scrolling window rather than displaying a message box for
 each.

File List: 
 UA_TIME\UA_TIME.DEF 
 UA_TIME\UA_TIME.ICO 
 UA_TIME\RESOURCE.H 
 UA_TIME\UA_TIME.MAK Makefile for 16-bit ua_time.exe 
 UA_TIME\UATIME32.MAK Makefile for 32-bit uatime32.exe 
 UA_TIME\UA_TIME.RC 
 UA_TIME\UA_TIME.C

------------------------------------------------------------------------
WAL.EXE: WinSock application launcher
-------------------------------------

This application was not referenced in the book at all It was
a tool created and used for exploration and development of many
programming techniques described throughout the text, however.
It's like a black box with many different knobs and displays.
It may seem overwhelming at first glance, but it is really quite
simple to use.

What it does: It can act as a client or server application,
 using TCP or UDP, in blocking, nonblocking or asynchronous operation
 mode, sending or receiving or both. It has many operational parameters,
 many of which can be adjusted on-the-fly during execution, and
 it allows setting and getting socket options as it executes also.
 It implements several simple protocols: echo (RFC 862), discard (RFC 
 863), and chargen (RFC 864).

What it illustrates: It is a WinSock exploration, experimentation
 and benchmarking tool that illustrates how a WinSock implementation
 behaves in certain conditions. Most significantly, it illustrates
 the effects of looping on recv() calls in response to asynchronous
 FD_READ event notification, and investigates how to adjust the
 read loop dynamically.

How to Use it: Typically, the first menu item to select
 is "Start," which displays the "Applications Options"
 dialog box. You can select the operation mode (blocking, non-blocking,
 or asynchronous), the transport protocol (TCP or UDP), and the
 application role (a client or server). If you choose to run as
 a client, you need to enter a destination host or address before
 you press the "Start!" button to initiate execution.
 There are a number of I/O parameters you can change by pressing
 the "Options" button..

  - The "Control" menu item has a pull-down menu that 
     allows you to open a new socket without starting a connection
     first. You can also close an existing connection gracefully, or
     abort it by resetting the connection.
  - The "Options" menu item has a pull-down menu with
     commands that change and examine socket options using setsockopt()
     and getsockopt(). You can also access the same application "I/O
     options" dialog available from the "Start" dialog.
     Any option can be changed "on-the-fly" as data is transferred
     during execution.
  - The "Statistics" menu item allows you to reset the
     counter values currently displayed in the main window to zero.
     The "Update" command forces an update of the statistics
     when the application is in a tight loop (i.e. when the timer messages
     that normally initiate update are not available).

Known Problems: The OOB implementation needs an application--which
 isn't available here--to receive the OOB data sent, and send some
 OOB data back. The detailed statics (avaliable from the "Statistics"
 menu item), causes the application to fail in the 32-bit version.
 It could benefit greatly from the addition of scripting and logging
 features. There are some application settings that can have problems
 during execution; for example, reading a constant stream of incoming
 data with a non-blocking socket (from the chargen port, for instance)
 can make the application unresponsive to user input.

 Also, running WAL as an echo server can have the same problems
 that AS_ECHO does when run against a client that sends faster
 than it receives. See the Known Problems section for AS_ECHO for
 more information.

File List: 
 WAL\ASYNC.C: Asynchronous operation mode routines
 WAL\BLOCKING.C: Blocking operation mode routines
 WAL\DEBUG.C:
 WAL\ERRNO.C 
 WAL\IDWALICO.ICO 
 WAL\NONBLKNG.C: Non-blocking operation mode routines 
 WAL\RESOURCE.H 
 WAL\WAL.MAK Makefile for 16-bit wal.exe 
 WAL\WAL32.MAK Makefile for 32-bit wal32.exe 
 WAL\OOBDATA.C: TCP out-of-band data routines
 WAL\WAL.DEF 
 WAL\WAL.H 
 WAL\WAL.INI Sample default settings file 
 WAL\WAL.RC 
 WAL\WAL.C

------------------------------------------------------------------------
WSASIMPL.DLL: sample DLL over WinSock
-------------------------------------

This DLL and the simple test program (SIMPLTST.EXE) are desribed
in Chapter 11, "DLLs over WinSock."

What it does: The WSASIMPL DLL rovides a simple API for
 elemental network functionality using TCP: connect, send or receive
 data, and disconnect.

What it illustrates: The main purpose of WSASIMPL is to
 illustrate how to create a DLL that runs over WinSock, and prevents
 reentrant messages from being sent to the application calling
 the DLL over WinSock. It uses windows subclassing to capture keyboard
 and mouse messages while blocking network operations are underway.

How to Use it: Here are short descriptions along with the
 prototypes for the functions in the simplified WinSock API that
 WSASIMPL.DLL provides:

SOCKET WINAPI ConnectTCP(LPSTR szDestination, LPSTR szService);
 ConnectTCP() calls the WinSock WSAStartup() function to register
 the task with the WinSock DLL, gets a socket handle with socket(),
 resolves the destination host name or formats an address using
 the GetAddr() function from the sample WINSOCKX library, and resolves
 the destination service or formats the port number with the WINSOCKX
 GetPort() function. When all these operations succeed, ConnectTCP()
 returns a socket handle; when any step fails the function returns
 INVALID_SOCKET.

int WINAPI SendData (SOCKET hSock, LPSTR lpOutBuf, int cbTotalToSend); 
 SendData() sends cbTotalToSend data bytes on the hSock socket
 handle, from the buffer lpOutBuf. When it succeeds, it returns
 the number of bytes sent, and when it fails it returns SOCKET_ERROR.

int WINAPI RecvData (SOCKET hSock, LPSTR lpInBuf, int cbTotalToRecv, int nTimeout); 
 RecvData() recieves up to cbTotalToRecv data bytes on the
 hSock socket handle, into the buffer lpInBuf, until the nTimeout
 period (in milliseconds) expires. It returns the number of bytes
 received on success, and SOCKET_ERROR on failure (timeout is an
 error condition).

int WINAPI CloseTCP (SOCKET hSock, LPSTR lpInBuf,int len);
 CloseTCP() is almost the same function as CloseConn() in the
 #WINSOCKX">WINSOCKX.LIB WinSock subroutine library.
 It attempts a graceful close of the TCP connection on socket hSock
 by calling shutdown() with how=1 to stop sends but allow
 receives, then it calls recv() to read data (up to len)
 into the buffer lpInBuf, if provided. When recv() fails with any
 error, or returns 0 to indicate closure from the remote, CloseTCP()
 then calls closesocket() to free the resources associated with
 the socket handle.

The SIMPLTST sample application connects to an echo server, sends
data, and reads it back. The relevant code for this application
is as follows:

    SOCKET hSock;
    char szHost[MAXHOSTNAME];
    char achOutBuf[BUFSIZE], achInBuf[BUFSIZE];

    hSock = ConnectTCP((LPSTR)szHost, (LPSTR)"echo");
    if (hSock != INVALID_SOCKET) {
        int nRet;
        SendData(hSock, achOutBuf, BUFSIZE);
        RecvData(hSock, achInBuf, BUFSIZE);
        CloseTCP(hSock, 0, 0);
    }

Known Problems: WSASIMPL.DLL subclasses the active window
 when ConnectTCP() is called to avoid reentrant messages. We avoid
 problems in our subclassed window procedure by noting receipt
 of a WM_DESTROY message. Also note that because WSASIMPL.DLL uses
 blocking operation mode, it imposes the typical limitations on
 each task (e.g. while any blocking operation is outstanding, other
 WinSock calls from the same task or thread fail with WSAEINPROGRESS).

File List: 
 WSASIMPL\SIMPLTST.DEF 
 WSASIMPL\SIMPLTST.ICO 
 WSASIMPL\SIMPLTST.RC 
 WSASIMPL\SIMPLTST.C 
 WSASIMPL\SIMPLTST.MAK Makefile for 16-bit simpltst.exe 
 WSASIMPL\SIMPLT32.MAK Makefile for 32-bit simplt32.exe 
 WSASIMPL\WSASIMPL.DEF 
 WSASIMPL\WSASIMPL.H 
 WSASIMPL\WSASIMPL.RC 
 WSASIMPL\WSASIMPL.C 
 WSASIMPL\WSASIMPL.MAK Makefile for 16-bit wsasimpl.dll 
 WSASIMPL\WSASMP32.MAK Makefile for 32-bit wsasmp32.dll

------------------------------------------------------------------------
WINSOCKX.LIB: WinSock subroutine library (static)
-------------------------------------------------

We describe each routine in this library in Chapter 7, "Sample
Application and Library." Many of the WinSock samples use the
subroutines in this library. For example, the error message box, the
routine to retrieve the local host's IP address, a simple hostname or
IP address string parser, and others.

What it does: The WINSOCKX static library contains a number of
 routines commonly used by WinSock applications. Some are non-network
 functions that perform common mundane application operations--such as
 centering a window, or creating a local file--but most of the
 functions in this library provide high-level WinSock services by
 combining a number of WinSock function calls. For example, you can
 pass either a host name or IP address string to our GetAddr()
 function, and it returns an IP address in network order (it uses
 inet_addr() and gethostbyname() WinSock functions).

What it illustrates: In effect, these reusable functions show how to
 write simpler and more robust WinSock application code by using
 high-level functions.

How to Use it: Here are the function prototypes of the most
 significant and useful routines, along with a short description of
 each:

int CloseConn (SOCKET hSock, LPSTR achInBuf, int len, HWND hWnd);</CODE> 
 This is the standard TCP close routine: shutdown(how=1), recv()
 loop, then closesocket().

u_long GetAddr (LPSTR szHost);
 The GetAddr() function takes either a host name or IP address
 string in standard Internet dot-notation, and returns a network
 addrss in network order.

int GetBuf (SOCKET hSock, int nBigBufSize, int nOptVal);
 This uses SO_RCVBUF or SO_SNDBUF to get largest buffer possible
 up to the upper limit set by nBigBufSize. Returns buffer
 size retreived from getsockopt() upon return on success, or SOCKET_ERROR
 on failure.

LONG GetHostID (void); 
 This uses gethostname(), then gethostbyname() to try to retrieve
 the local IP address, and if this fails it then gets a UDP socket,
 connects it, and uses getsockname() to retrieve the local IP address,
 before closing the socket.

u_short GetPort (LPSTR szService); 
 This function is analogous to our GetAddr(), but takes either
 a service name or port number string, and returns a port number
 in network order.

int WSAErrStr (int WSAErr, LPSTR lpErrBuf);
 WSAErrStr() takes a WinSock error value as an input argument,
 and returns a short error description in the buffer provided.

void WSAperror (int WSAErr, LPSTR szFuncName);
 This function displays a message box that indicates "szFuncName
 failed," displays the macro name for the WSAErr value,
 along with a short description of the error.

Known Problems: No known problems.

File List: 
 WINSOCKX\GLOBALS.C global variables 
 WINSOCKX\WIN_MGR.C (non-network related) windows routines 
 WINSOCKX\WINSOCKX.MAK Makefile for 16-bit winsockx.lib 
 WINSOCKX\WSOCK32X.MAK Makefile for 32-bit wsock32x.lib 
 WINSOCKX\WSA_ERR.C error routines 
 WINSOCKX\WSA_OPTN.C socket option routines 
 WINSOCKX\WSA_ADDR.C address routines

-- end of file --
