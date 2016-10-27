# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "WSOCK32X.MAK" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : .\WinRel\WSOCK32X.lib .\WinRel\WSOCK32X.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"WSOCK32X.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"WSOCK32X.bsc" 
BSC32_SBRS= \
	.\WinRel\WIN_MGR.SBR \
	.\WinRel\WSA_ERR.SBR \
	.\WinRel\GLOBALS.SBR \
	.\WinRel\WSA_OPTN.SBR \
	.\WinRel\WSA_ADDR.SBR \
	.\WinRel\ABOUT.SBR \
	.\WinRel\CLOSECON.SBR \
        .\WinRel\FILE_DIR.SBR

.\WinRel\WSOCK32X.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO
LIB32_FLAGS=/NOLOGO /OUT:$(OUTDIR)\"WSOCK32X.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	.\WinRel\WIN_MGR.OBJ \
	.\WinRel\WSA_ERR.OBJ \
	.\WinRel\GLOBALS.OBJ \
	.\WinRel\WSA_OPTN.OBJ \
	.\WinRel\WSA_ADDR.OBJ \
	.\WinRel\ABOUT.OBJ \
	.\WinRel\CLOSECON.OBJ \
        .\WinRel\FILE_DIR.OBJ

.\WinRel\WSOCK32X.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : .\WinDebug\WSOCK32X.lib .\WinDebug\WSOCK32X.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"WSOCK32X.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinDebug/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"WSOCK32X.bsc" 
BSC32_SBRS= \
	.\WinDebug\WIN_MGR.SBR \
	.\WinDebug\WSA_ERR.SBR \
	.\WinDebug\GLOBALS.SBR \
	.\WinDebug\WSA_OPTN.SBR \
	.\WinDebug\WSA_ADDR.SBR \
	.\WinDebug\ABOUT.SBR \
	.\WinDebug\CLOSECON.SBR \
        .\WinDebug\FILE_DIR.SBR

.\WinDebug\WSOCK32X.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO
LIB32_FLAGS=/NOLOGO /OUT:$(OUTDIR)\"WSOCK32X.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	.\WinDebug\WIN_MGR.OBJ \
	.\WinDebug\WSA_ERR.OBJ \
	.\WinDebug\GLOBALS.OBJ \
	.\WinDebug\WSA_OPTN.OBJ \
	.\WinDebug\WSA_ADDR.OBJ \
	.\WinDebug\ABOUT.OBJ \
	.\WinDebug\CLOSECON.OBJ \
        .\WinDebug\FILE_DIR.OBJ

.\WinDebug\WSOCK32X.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\WIN_MGR.C

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\WIN_MGR.OBJ :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\WIN_MGR.OBJ :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WSA_ERR.C
DEP_WSA_E=\
	..\WSA_XTRA.H\
	..\WINSOCKX.H

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\WSA_ERR.OBJ :  $(SOURCE)  $(DEP_WSA_E) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\WSA_ERR.OBJ :  $(SOURCE)  $(DEP_WSA_E) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GLOBALS.C
DEP_GLOBA=\
	..\WINSOCKX.H\
	..\WSA_XTRA.H

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\GLOBALS.OBJ :  $(SOURCE)  $(DEP_GLOBA) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\GLOBALS.OBJ :  $(SOURCE)  $(DEP_GLOBA) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WSA_OPTN.C
DEP_WSA_O=\
	..\WSA_XTRA.H\
	..\WINSOCKX.H

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\WSA_OPTN.OBJ :  $(SOURCE)  $(DEP_WSA_O) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\WSA_OPTN.OBJ :  $(SOURCE)  $(DEP_WSA_O) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WSA_ADDR.C
DEP_WSA_A=\
	..\WSA_XTRA.H

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\WSA_ADDR.OBJ :  $(SOURCE)  $(DEP_WSA_A) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\WSA_ADDR.OBJ :  $(SOURCE)  $(DEP_WSA_A) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ABOUT.C
DEP_ABOUT=\
	..\WINSOCKX.H

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\ABOUT.OBJ :  $(SOURCE)  $(DEP_ABOUT) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\ABOUT.OBJ :  $(SOURCE)  $(DEP_ABOUT) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CLOSECON.C
DEP_CLOSE=\
	..\WINSOCKX.H

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\CLOSECON.OBJ :  $(SOURCE)  $(DEP_CLOSE) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\CLOSECON.OBJ :  $(SOURCE)  $(DEP_CLOSE) $(INTDIR)

!ENDIF 

# End Source File
# End Group
# End Project
################################################################################
