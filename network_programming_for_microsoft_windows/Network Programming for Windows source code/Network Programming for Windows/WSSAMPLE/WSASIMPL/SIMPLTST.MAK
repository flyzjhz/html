# Microsoft Visual C++ generated build script - Do not modify

PROJ = SIMPLTST
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WINSOCK\WSASIMPL\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = SIMPLTST.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W4 /Zi /AL /Od /D "_DEBUG" /FR /GA /Fd"SIMPLTST.PDB"
CFLAGS_R_WEXE = /nologo /W3 /AM /O1 /D "NDEBUG" /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:32768 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = oldnames libw llibcew commdlg.lib olecli.lib olesvr.lib shell.lib 
LIBS_R_WEXE = oldnames libw mlibcew commdlg.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = SIMPLTST.DEF
OBJS_EXT = 
LIBS_EXT = WSASIMPL.LIB ..\WINSOCKX.LIB 
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
SBRS = SIMPLTST.SBR


WSASIMPL_DEP = 

SIMPLTST_DEP = c:\winsock\wsasimpl\wsasimpl.h \
	c:\2.2\include\winsock.h \
	c:\winsock\winsockx.h \
	c:\winsock\wsa_xtra.h


SIMPLTST_RCDEP = c:\winsock\wsasimpl\simpltst.ico


WINSOCKX_DEP = 

all:	$(PROJ).EXE $(PROJ).BSC

SIMPLTST.OBJ:	SIMPLTST.C $(SIMPLTST_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c SIMPLTST.C

SIMPLTST.RES:	SIMPLTST.RC $(SIMPLTST_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r SIMPLTST.RC


$(PROJ).EXE::	SIMPLTST.RES

$(PROJ).EXE::	SIMPLTST.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
SIMPLTST.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\2.2\lib\+
c:\msvc\lib\+
c:\msvc\mfc\lib\+
WSASIMPL.LIB+
..\WINSOCKX.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) SIMPLTST.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	SIMPLTST.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) SIMPLTST.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
