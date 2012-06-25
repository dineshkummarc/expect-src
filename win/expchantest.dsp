# Microsoft Developer Studio Project File - Name="expchantest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=expchantest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "expchantest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "expchantest.mak" CFG="expchantest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "expchantest - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "expchantest - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "expchantest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "expchantest___Win32_Release"
# PROP BASE Intermediate_Dir "expchantest___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\expchantest"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXPCHANTEST_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "d:\tcl_workspace\tcl_84_branch\generic" /I ".\mcl" /I "..\generic" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "USE_TCL_STUBS" /D "BUILD_exp" /D __CMCL_THROW_EXCEPTIONS__=0 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\generic" /i "d:\tcl_workspace\tcl_84_branch\generic" /i "." /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib /nologo /dll /machine:I386 /out:"Release/expect543.dll" /libpath:"d:\tcl_workspace\tcl_84_branch\win\Release" /release
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\expchantest"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXPCHANTEST_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "d:\tcl_workspace\tcl_84_branch\generic" /I ".\mcl" /I "..\generic" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "USE_TCL_STUBS" /D "BUILD_exp" /D __CMCL_THROW_EXCEPTIONS__=0 /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\generic" /i "d:\tcl_workspace\tcl_84_branch\generic" /i "." /d "DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib /nologo /dll /debug /machine:I386 /out:"Debug/expect543.dll" /pdbtype:sept /libpath:"d:\tcl_workspace\tcl_84_branch\win\Release"

!ENDIF 

# Begin Target

# Name "expchantest - Win32 Release"
# Name "expchantest - Win32 Debug"
# Begin Group "mcl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Mcl\CMcl.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclAutoLock.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclAutoLock.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclAutoPtr.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclAutoPtr.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclCritSec.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclCritSec.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclEvent.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclGlobal.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclGlobal.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclKernel.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclKernel.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclLinkedLists.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclMailbox.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclMailbox.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclMonitor.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclMonitor.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclMutex.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclSemaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclSemaphore.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclSharedMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclSharedMemory.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclThread.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclThread.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclWaitableCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclWaitableCollection.h
# End Source File
# Begin Source File

SOURCE=.\Mcl\CMclWaitableObject.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\generic\exp.decls
# End Source File
# Begin Source File

SOURCE=..\generic\exp.h
# End Source File
# Begin Source File

SOURCE=.\expchantest.c
# End Source File
# Begin Source File

SOURCE=.\expchantest.h
# End Source File
# Begin Source File

SOURCE=..\generic\expDecls.h
# End Source File
# Begin Source File

SOURCE=.\expect.rc

!IF  "$(CFG)" == "expchantest - Win32 Release"

!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"

# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\generic\expInt.h
# End Source File
# Begin Source File

SOURCE=..\generic\expIntDecls.h
# End Source File
# Begin Source File

SOURCE=..\generic\expIntPlatDecls.h
# End Source File
# Begin Source File

SOURCE=..\generic\expPlatDecls.h
# End Source File
# Begin Source File

SOURCE=.\expWinConsoleDebugger.cpp
# End Source File
# Begin Source File

SOURCE=.\expWinConsoleDebugger.hpp
# End Source File
# Begin Source File

SOURCE=.\expWinConsoleDebuggerBreakPoints.cpp
# End Source File
# Begin Source File

SOURCE=.\expWinErr.h
# End Source File
# Begin Source File

SOURCE=.\expWinErr.mc

!IF  "$(CFG)" == "expchantest - Win32 Release"

# Begin Custom Build - Compiling message catalog...
InputPath=.\expWinErr.mc

BuildCmds= \
	mc -c -U -w $(InputPath)

".\MSG00409.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\expWinErr.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\expWinErr.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"

# Begin Custom Build - Compiling message catalog...
InputPath=.\expWinErr.mc

BuildCmds= \
	mc -c -U -w $(InputPath)

".\MSG00409.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\expWinErr.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\expWinErr.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\expWinInit.c
# End Source File
# Begin Source File

SOURCE=.\expWinInjectorIPC.hpp
# End Source File
# Begin Source File

SOURCE=.\expWinMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\expWinMessage.hpp
# End Source File
# Begin Source File

SOURCE=.\expWinPort.h
# End Source File
# Begin Source File

SOURCE=.\expWinSpawnChan.cpp
# End Source File
# Begin Source File

SOURCE=.\expWinUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\TclHash.hpp
# End Source File
# End Target
# End Project
