#
# File: winsockx.mak
#
# Description: makefile for sample library
#
# Library Contents:
#   about.c:    Common About dialog routine
#   closecon.c: TCP close routine
#   globals.c:  Variables common to library and users
#   file_dir.c: File and directory related routines
#   win_mgr.c:  Window manager rouines
#   wsa_addr.c: Common address routines
#   wsa_err.c:  WinSock error string retrieval and display
#   wsa_optn.c: Common socket option functions
#

DEBUG=DEBUG

OBJS = GLOBALS.OBJ  WIN_MGR.OBJ WSA_ADDR.OBJ WSA_ERR.OBJ \
       WSA_OPTN.OBJ ABOUT.OBJ   CLOSECON.OBJ FILE_DIR.OBJ
LIBS = LIBW        

!if "$(DEBUG)" == "DEBUG"
C_DEBUG = -Zi -Od 
L_DEBUG = /CO
!endif

CFLAGS = -DWINVER=0x030a $(C_DEBUG) -ALw -Ow -Gsw -Zpe -Zl -W4
LFLAGS	= /NOE/NOD/MAP $(L_DEBUG)

all: winsockx.lib

winsockx.lib: $(OBJS)
    del  winsockx.lib
    lib  winsockx.lib+$(OBJS);
    copy winsockx.lib ..

$(OBJS): ..\wsa_xtra.h ..\winsockx.h

