# Microsoft Visual C++ generated build script - Do not modify

PROJ = WAL
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WINSOCK\WAL\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = WAL.C       
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W4 /Zi /AM /Od /D "_DEBUG" /FR /GA /Fd"WAL.PDB"
CFLAGS_R_WEXE = /nologo /W4 /AM /O1 /D "NDEBUG" /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO /MAP /LINE  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:5120 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = oldnames libw mlibcew commdlg.lib olecli.lib olesvr.lib shell.lib toolhelp.lib 
LIBS_R_WEXE = oldnames libw mlibcew commdlg.lib olecli.lib olesvr.lib shell.lib toolhelp.lib 
RCFLAGS = /nologo 
RESFLAGS = /nologo 
RUNFLAGS = 
DEFFILE = WAL.DEF
OBJS_EXT = 
LIBS_EXT = ..\WINSOCK.LIB ..\WINSOCKX.LIB 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = WAL.SBR \
		BLOCKING.SBR \
		NONBLKNG.SBR \
		ASYNC.SBR \
		OOBDATA.SBR \
		DEBUG.SBR


WAL_DEP = c:\winsock\wal\wal.h \
	c:\2.2\include\winsock.h


WAL_RCDEP = c:\winsock\wal\wal.h \
	c:\2.2\include\winsock.h \
	c:\winsock\wal\idwalico.ico


BLOCKING_DEP = c:\2.2\include\winsock.h \
	c:\winsock\wal\wal.h


NONBLKNG_DEP = c:\2.2\include\winsock.h \
	c:\winsock\wal\wal.h


ASYNC_DEP = c:\2.2\include\winsock.h \
	c:\winsock\wal\wal.h


OOBDATA_DEP = c:\2.2\include\winsock.h \
	c:\winsock\wal\wal.h


DEBUG_DEP = c:\winsock\wal\wal.h \
	c:\2.2\include\winsock.h


WINSOCK_DEP = 

WINSOCKX_DEP = 

all:	$(PROJ).EXE $(PROJ).BSC

WAL.OBJ:	WAL.C $(WAL_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c WAL.C

WAL.RES:	WAL.RC $(WAL_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r WAL.RC

BLOCKING.OBJ:	BLOCKING.C $(BLOCKING_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c BLOCKING.C

NONBLKNG.OBJ:	NONBLKNG.C $(NONBLKNG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c NONBLKNG.C

ASYNC.OBJ:	ASYNC.C $(ASYNC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ASYNC.C

OOBDATA.OBJ:	OOBDATA.C $(OOBDATA_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c OOBDATA.C

DEBUG.OBJ:	DEBUG.C $(DEBUG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DEBUG.C


$(PROJ).EXE::	WAL.RES

$(PROJ).EXE::	WAL.OBJ BLOCKING.OBJ NONBLKNG.OBJ ASYNC.OBJ OOBDATA.OBJ DEBUG.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
WAL.OBJ +
BLOCKING.OBJ +
NONBLKNG.OBJ +
ASYNC.OBJ +
OOBDATA.OBJ +
DEBUG.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\2.2\lib\+
c:\msvc\lib\+
c:\msvc\mfc\lib\+
..\WINSOCK.LIB+
..\WINSOCKX.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) WAL.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	WAL.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) WAL.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
