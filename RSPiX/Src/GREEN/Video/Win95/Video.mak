# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=Exe Debug
!MESSAGE No configuration specified.  Defaulting to Exe Debug.
!ENDIF 

!IF "$(CFG)" != "Exe Release" && "$(CFG)" != "Exe Debug" && "$(CFG)" !=\
 "Lib Debug" && "$(CFG)" != "Lib Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Video.mak" CFG="Exe Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Exe Release" (based on "Win32 (x86) Application")
!MESSAGE "Exe Debug" (based on "Win32 (x86) Application")
!MESSAGE "Lib Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Lib Release" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Exe Debug"

!IF  "$(CFG)" == "Exe Release"

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

ALL : $(OUTDIR)/Video.exe $(OUTDIR)/Video.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /YX /O2 /I "....\blue" /I "....\green\hot" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /YX /O2 /I "....\blue" /I "....\green\hot" /D "WIN32"\
 /D "NDEBUG" /D "_WINDOWS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"Video.pch" /Fo$(INTDIR)/\
 /c 
CPP_OBJS=.\WinRel/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"Video.bsc" 
BSC32_SBRS= \
	$(INTDIR)/VIDEO.SBR \
	$(INTDIR)/hot.sbr \
	$(INTDIR)/sampapp.sbr

$(OUTDIR)/Video.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO\
 /SUBSYSTEM:windows /INCREMENTAL:no /PDB:$(OUTDIR)/"Video.pdb" /MACHINE:I386\
 /OUT:$(OUTDIR)/"Video.exe" 
DEF_FILE=
LINK32_OBJS= \
	\W\SRC\RSPiX\BLUE\WinBlue.lib \
	$(INTDIR)/VIDEO.OBJ \
	$(INTDIR)/hot.obj \
	$(INTDIR)/sampapp.obj \
	\C\MSTOOLS\lib\VFW32.LIB

$(OUTDIR)/Video.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Exe Debug"

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

ALL : $(OUTDIR)/Video.exe $(OUTDIR)/Video.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /Zi /YX /Od /I "....\blue" /I "....\green\hot" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /Zi /YX /Od /I "....\blue" /I "....\green\hot" /D\
 "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"Video.pch"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"Video.pdb" /c 
CPP_OBJS=.\WinDebug/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"Video.bsc" 
BSC32_SBRS= \
	$(INTDIR)/VIDEO.SBR \
	$(INTDIR)/hot.sbr \
	$(INTDIR)/sampapp.sbr

$(OUTDIR)/Video.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO\
 /SUBSYSTEM:windows /INCREMENTAL:yes /PDB:$(OUTDIR)/"Video.pdb" /DEBUG\
 /MACHINE:I386 /OUT:$(OUTDIR)/"Video.exe" 
DEF_FILE=
LINK32_OBJS= \
	\W\SRC\RSPiX\BLUE\WinBlueD.lib \
	$(INTDIR)/VIDEO.OBJ \
	$(INTDIR)/hot.obj \
	$(INTDIR)/sampapp.obj \
	\C\MSTOOLS\lib\VFW32.LIB

$(OUTDIR)/Video.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Lib Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Lib_Debu"
# PROP BASE Intermediate_Dir "Lib_Debu"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Lib_Debu"
# PROP Intermediate_Dir "Lib_Debu"
OUTDIR=.\Lib_Debu
INTDIR=.\Lib_Debu

ALL : .\VideoD.lib $(OUTDIR)/Video.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /Z7 /YX /Od /I "....\blue" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /Z7 /YX /Od /I "....\blue" /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"Video.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\Lib_Debu/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"Video.bsc" 
BSC32_SBRS= \
	$(INTDIR)/VIDEO.SBR

$(OUTDIR)/Video.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO /OUT:"VideoD.lib"
LIB32_FLAGS=/NOLOGO /OUT:"VideoD.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	$(INTDIR)/VIDEO.OBJ \
	\C\MSTOOLS\lib\VFW32.LIB

.\VideoD.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Lib Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Lib_Rele"
# PROP BASE Intermediate_Dir "Lib_Rele"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Lib_Rele"
# PROP Intermediate_Dir "Lib_Rele"
OUTDIR=.\Lib_Rele
INTDIR=.\Lib_Rele

ALL : .\Video.lib $(OUTDIR)/Video.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /YX /O2 /I "....\blue" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /YX /O2 /I "....\blue" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"Video.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\Lib_Rele/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"Video.bsc" 
BSC32_SBRS= \
	$(INTDIR)/VIDEO.SBR

$(OUTDIR)/Video.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO /OUT:"Video.lib"
LIB32_FLAGS=/NOLOGO /OUT:"Video.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	$(INTDIR)/VIDEO.OBJ \
	\C\MSTOOLS\lib\VFW32.LIB

.\Video.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Group "Blue Library"

################################################################################
# Begin Source File

SOURCE=\W\SRC\RSPiX\BLUE\WinBlue.lib

!IF  "$(CFG)" == "Exe Release"

!ELSEIF  "$(CFG)" == "Exe Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Lib Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Lib Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\W\SRC\RSPiX\BLUE\WinBlueD.lib

!IF  "$(CFG)" == "Exe Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exe Debug"

!ELSEIF  "$(CFG)" == "Lib Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Lib Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
################################################################################
# Begin Group "Green"

################################################################################
# Begin Source File

SOURCE=.\VIDEO.CPP
DEP_VIDEO=\
	\W\SRC\RSPiX\BLUE\BLUE.H\
	\W\SRC\RSPiX\BLUE\BLUEWIN.H\
	.\VIDEO.H\
	\W\SRC\RSPiX\BLUE\SYSTEM.H\
	\W\SRC\RSPiX\BLUE\BDISPLAY.H\
	\W\SRC\RSPiX\BLUE\BPALETTE.H\
	\W\SRC\RSPiX\BLUE\BSOUND.H\
	\W\SRC\RSPiX\BLUE\BMIDI.H\
	\W\SRC\RSPiX\BLUE\BMOUSE.H\
	\W\SRC\RSPiX\BLUE\BKEY.H\
	\W\SRC\RSPiX\BLUE\BMAIN.H\
	\W\SRC\RSPiX\BLUE\BDEBUG.H\
	\W\SRC\RSPiX\BLUE\BTIME.H\
	\W\SRC\RSPiX\BLUE\BJOY.H\
	\W\SRC\RSPiX\BLUE\BDIALOG.H\
	\W\SRC\RSPiX\BLUE\BCRITIC.H\
	\W\SRC\RSPiX\BLUE\BPARMS.H\
	\W\SRC\RSPiX\BLUE\WIN.H

!IF  "$(CFG)" == "Exe Release"

$(INTDIR)/VIDEO.OBJ :  $(SOURCE)  $(DEP_VIDEO) $(INTDIR)

!ELSEIF  "$(CFG)" == "Exe Debug"

$(INTDIR)/VIDEO.OBJ :  $(SOURCE)  $(DEP_VIDEO) $(INTDIR)

!ELSEIF  "$(CFG)" == "Lib Debug"

$(INTDIR)/VIDEO.OBJ :  $(SOURCE)  $(DEP_VIDEO) $(INTDIR)

!ELSEIF  "$(CFG)" == "Lib Release"

$(INTDIR)/VIDEO.OBJ :  $(SOURCE)  $(DEP_VIDEO) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\W\SRC\RSPiX\Green\Hot\hot.cpp
DEP_HOT_C=\
	\W\SRC\RSPiX\BLUE\SYSTEM.H\
	\W\SRC\RSPiX\BLUE\BLUEWIN.H\
	\W\SRC\RSPiX\BLUE\BDEBUG.H\
	\W\SRC\RSPiX\Green\Hot\hot.h\
	\W\SRC\RSPiX\BLUE\WIN.H

!IF  "$(CFG)" == "Exe Release"

$(INTDIR)/hot.obj :  $(SOURCE)  $(DEP_HOT_C) $(INTDIR)
   $(CPP) /nologo /W3 /GX /YX /O2 /I "....\blue" /I "....\green\hot" /D "WIN32"\
 /D "NDEBUG" /D "_WINDOWS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"Video.pch" /Fo$(INTDIR)/\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Exe Debug"

$(INTDIR)/hot.obj :  $(SOURCE)  $(DEP_HOT_C) $(INTDIR)
   $(CPP) /nologo /W3 /GX /Zi /YX /Od /I "....\blue" /I "....\green\hot" /D\
 "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"Video.pch"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"Video.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Lib Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Lib Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
################################################################################
# Begin Group "Yellow"

################################################################################
# Begin Source File

SOURCE=.\sampapp.cpp
DEP_SAMPA=\
	\W\SRC\RSPiX\BLUE\SYSTEM.H\
	\W\SRC\RSPiX\BLUE\BLUE.H\
	.\VIDEO.H\
	\W\SRC\RSPiX\Green\Hot\hot.h\
	\W\SRC\RSPiX\blue\wdisplay.h\
	\W\SRC\RSPiX\BLUE\BDISPLAY.H\
	\W\SRC\RSPiX\BLUE\BPALETTE.H\
	\W\SRC\RSPiX\BLUE\BSOUND.H\
	\W\SRC\RSPiX\BLUE\BMIDI.H\
	\W\SRC\RSPiX\BLUE\BMOUSE.H\
	\W\SRC\RSPiX\BLUE\BKEY.H\
	\W\SRC\RSPiX\BLUE\BMAIN.H\
	\W\SRC\RSPiX\BLUE\BDEBUG.H\
	\W\SRC\RSPiX\BLUE\BTIME.H\
	\W\SRC\RSPiX\BLUE\BJOY.H\
	\W\SRC\RSPiX\BLUE\BDIALOG.H\
	\W\SRC\RSPiX\BLUE\BCRITIC.H\
	\W\SRC\RSPiX\BLUE\BPARMS.H\
	\W\SRC\RSPiX\BLUE\WIN.H

!IF  "$(CFG)" == "Exe Release"

$(INTDIR)/sampapp.obj :  $(SOURCE)  $(DEP_SAMPA) $(INTDIR)
   $(CPP) /nologo /W3 /GX /YX /O2 /I "....\blue" /I "....\green\hot" /D "WIN32"\
 /D "NDEBUG" /D "_WINDOWS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"Video.pch" /Fo$(INTDIR)/\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Exe Debug"

$(INTDIR)/sampapp.obj :  $(SOURCE)  $(DEP_SAMPA) $(INTDIR)
   $(CPP) /nologo /W3 /GX /Zi /YX /Od /I "....\blue" /I "....\green\hot" /D\
 "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"Video.pch"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"Video.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Lib Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Lib Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
################################################################################
# Begin Group "AdLib"

################################################################################
# Begin Source File

SOURCE=\C\MSTOOLS\lib\VFW32.LIB

!IF  "$(CFG)" == "Exe Release"

!ELSEIF  "$(CFG)" == "Exe Debug"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "Lib Debug"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "Lib Release"

!ENDIF 

# End Source File
# End Group
# End Project
################################################################################
