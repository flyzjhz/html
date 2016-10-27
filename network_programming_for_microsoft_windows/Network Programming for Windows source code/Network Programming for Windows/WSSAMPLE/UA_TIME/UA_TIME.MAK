# Microsoft Visual C++ generated build script - Do not modify

PROJ = UA_TIME
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WINSOCK\UA_TIME\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = UA_TIME.C   
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W4 /Zi /AM /Od /D "_DEBUG" /FR /GA /Fd"UA_TIME.PDB"
CFLAGS_R_WEXE = /nologo /W3 /AM /O1 /D "NDEBUG" /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /CO /ALIGN:16 /STACK:10240
LFLAGS_R_WEXE = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /ALIGN:16 /STACK:10240
LIBS_D_WEXE = oldnames libw commdlg shell olecli olesvr mlibcew
LIBS_R_WEXE = oldnames libw commdlg shell olecli olesvr mlibcew
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = UA_TIME.DEF
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
SBRS = UA_TIME.SBR


WINSOCK_DEP = 

WINSOCKX_DEP = 

UA_TIME_DEP = c:\2.2\include\winsock.h \
	c:\winsock\winsockx.h \
	c:\winsock\wsa_xtra.h


UA_TIME_RCDEP = c:\2.2\include\winsock.h \
	c:\winsock\ua_time\ua_time.ico


all:	$(PROJ).EXE $(PROJ).BSC

UA_TIME.OBJ:	UA_TIME.C $(UA_TIME_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c UA_TIME.C

UA_TIME.RES:	UA_TIME.RC $(UA_TIME_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r UA_TIME.RC


$(PROJ).EXE::	UA_TIME.RES

$(PROJ).EXE::	UA_TIME.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
UA_TIME.OBJ +
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
	$(RC) $(RESFLAGS) UA_TIME.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	UA_TIME.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) UA_TIME.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
