# Microsoft Developer Studio Project File - Name="injector" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=injector - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "injector.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "injector.mak" CFG="injector - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "injector - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "injector - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "injector - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "injector___Win32_Release"
# PROP BASE Intermediate_Dir "injector___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\injector"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "INJECTOR_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\mcl" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D __CMCL_THROW_EXCEPTIONS__=0 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\generic" /i "d:\tcl_workspace\tcl_84_branch\generic" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib /nologo /subsystem:console /dll /machine:I386 /release /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "injector - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "injector___Win32_Debug"
# PROP BASE Intermediate_Dir "injector___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\injector"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "INJECTOR_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I ".\mcl" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D __CMCL_THROW_EXCEPTIONS__=0 /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\generic" /i "d:\tcl_workspace\tcl_84_branch\generic" /d "DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "injector - Win32 Release"
# Name "injector - Win32 Debug"
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

SOURCE=.\expWinInjectorIPC.hpp
# End Source File
# Begin Source File

SOURCE=.\expWinInjectorMain.cpp
# End Source File
# Begin Source File

SOURCE=.\inject.rc

!IF  "$(CFG)" == "injector - Win32 Release"

!ELSEIF  "$(CFG)" == "injector - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
