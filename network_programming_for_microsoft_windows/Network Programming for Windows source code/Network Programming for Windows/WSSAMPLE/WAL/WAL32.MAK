# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "wal32.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

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

ALL : $(OUTDIR)/wal32.exe $(OUTDIR)/wal32.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"wal32.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"WAL.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"wal32.bsc" 
BSC32_SBRS= \
	$(INTDIR)/OOBDATA.SBR \
	$(INTDIR)/ASYNC.SBR \
	$(INTDIR)/DEBUG.SBR \
	$(INTDIR)/WAL.SBR \
	$(INTDIR)/NONBLKNG.SBR \
	$(INTDIR)/BLOCKING.SBR

$(OUTDIR)/wal32.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO\
 /SUBSYSTEM:windows /INCREMENTAL:no /PDB:$(OUTDIR)/"wal32.pdb" /MACHINE:I386\
 /DEF:".\WAL32.DEF" /OUT:$(OUTDIR)/"wal32.exe" 
DEF_FILE=.\WAL32.DEF
LINK32_OBJS= \
	$(INTDIR)/OOBDATA.OBJ \
	$(INTDIR)/ASYNC.OBJ \
	$(INTDIR)/WAL.res \
	$(INTDIR)/DEBUG.OBJ \
	$(INTDIR)/WAL.OBJ \
	$(INTDIR)/NONBLKNG.OBJ \
	$(INTDIR)/BLOCKING.OBJ \
	\WINSOCK\WSOCK32.LIB \
	\WINSOCK\WSOCK32X.lib

$(OUTDIR)/wal32.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
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

ALL : $(OUTDIR)/wal32.exe $(OUTDIR)/wal32.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"wal32.pch" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"wal32.pdb"\
 /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"WAL.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"wal32.bsc" 
BSC32_SBRS= \
	$(INTDIR)/OOBDATA.SBR \
	$(INTDIR)/ASYNC.SBR \
	$(INTDIR)/DEBUG.SBR \
	$(INTDIR)/WAL.SBR \
	$(INTDIR)/NONBLKNG.SBR \
	$(INTDIR)/BLOCKING.SBR

$(OUTDIR)/wal32.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO\
 /SUBSYSTEM:windows /INCREMENTAL:yes /PDB:$(OUTDIR)/"wal32.pdb" /DEBUG\
 /MACHINE:I386 /DEF:".\WAL32.DEF" /OUT:$(OUTDIR)/"wal32.exe" 
DEF_FILE=.\WAL32.DEF
LINK32_OBJS= \
	$(INTDIR)/OOBDATA.OBJ \
	$(INTDIR)/ASYNC.OBJ \
	$(INTDIR)/WAL.res \
	$(INTDIR)/DEBUG.OBJ \
	$(INTDIR)/WAL.OBJ \
	$(INTDIR)/NONBLKNG.OBJ \
	$(INTDIR)/BLOCKING.OBJ \
	\WINSOCK\WSOCK32.LIB \
	\WINSOCK\WSOCK32X.lib

$(OUTDIR)/wal32.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
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

SOURCE=.\OOBDATA.C
DEP_OOBDA=\
	.\WAL.H

$(INTDIR)/OOBDATA.OBJ :  $(SOURCE)  $(DEP_OOBDA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ASYNC.C
DEP_ASYNC=\
	.\WAL.H

$(INTDIR)/ASYNC.OBJ :  $(SOURCE)  $(DEP_ASYNC) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WAL.RC
DEP_WAL_R=\
	.\IDWALICO.ICO\
	.\WAL.H

$(INTDIR)/WAL.res :  $(SOURCE)  $(DEP_WAL_R) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DEBUG.C
DEP_DEBUG=\
	.\WAL.H

$(INTDIR)/DEBUG.OBJ :  $(SOURCE)  $(DEP_DEBUG) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WAL.C
DEP_WAL_C=\
	.\WAL.H

$(INTDIR)/WAL.OBJ :  $(SOURCE)  $(DEP_WAL_C) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\NONBLKNG.C
DEP_NONBL=\
	.\WAL.H

$(INTDIR)/NONBLKNG.OBJ :  $(SOURCE)  $(DEP_NONBL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BLOCKING.C
DEP_BLOCK=\
	.\WAL.H

$(INTDIR)/BLOCKING.OBJ :  $(SOURCE)  $(DEP_BLOCK) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=\WINSOCK\WSOCK32.LIB
# End Source File
################################################################################
# Begin Source File

SOURCE=\WINSOCK\WSOCK32X.lib
# End Source File
################################################################################
# Begin Source File

SOURCE=.\WAL32.DEF
# End Source File
# End Group
# End Project
################################################################################
