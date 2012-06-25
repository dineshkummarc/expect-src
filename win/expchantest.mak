# Microsoft Developer Studio Generated NMAKE File, Based on expchantest.dsp
!IF "$(CFG)" == ""
CFG=expchantest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to expchantest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "expchantest - Win32 Release" && "$(CFG)" != "expchantest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "expchantest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release\expchantest
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\expect540.dll"


CLEAN :
	-@erase "$(INTDIR)\CMclAutoLock.obj"
	-@erase "$(INTDIR)\CMclAutoPtr.obj"
	-@erase "$(INTDIR)\CMclCritSec.obj"
	-@erase "$(INTDIR)\CMclEvent.obj"
	-@erase "$(INTDIR)\CMclGlobal.obj"
	-@erase "$(INTDIR)\CMclKernel.obj"
	-@erase "$(INTDIR)\CMclMailbox.obj"
	-@erase "$(INTDIR)\CMclMonitor.obj"
	-@erase "$(INTDIR)\CMclMutex.obj"
	-@erase "$(INTDIR)\CMclSemaphore.obj"
	-@erase "$(INTDIR)\CMclSharedMemory.obj"
	-@erase "$(INTDIR)\CMclThread.obj"
	-@erase "$(INTDIR)\CMclWaitableCollection.obj"
	-@erase "$(INTDIR)\expchantest.obj"
	-@erase "$(INTDIR)\expect.res"
	-@erase "$(INTDIR)\expWinConsoleDebugger.obj"
	-@erase "$(INTDIR)\expWinConsoleDebuggerBreakPoints.obj"
	-@erase "$(INTDIR)\expWinInit.obj"
	-@erase "$(INTDIR)\expWinMessage.obj"
	-@erase "$(INTDIR)\expWinSpawnChan.obj"
	-@erase "$(INTDIR)\expWinUtils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\expect540.dll"
	-@erase "$(OUTDIR)\expect540.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\tcl_workspace\tcl_84_branch\generic" /I ".\mcl" /I "..\generic" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "USE_TCL_STUBS" /D "BUILD_exp" /D __CMCL_THROW_EXCEPTIONS__=0 /Fp"$(INTDIR)\expchantest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\expect.res" /i "..\generic" /i "d:\tcl_workspace\tcl_84_branch\generic" /i "." /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\expchantest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\expect540.pdb" /machine:I386 /out:"$(OUTDIR)\expect540.dll" /implib:"$(OUTDIR)\expect540.lib" /libpath:"d:\tcl_workspace\tcl_84_branch\win\Release" /release 
LINK32_OBJS= \
	"$(INTDIR)\CMclAutoLock.obj" \
	"$(INTDIR)\CMclAutoPtr.obj" \
	"$(INTDIR)\CMclCritSec.obj" \
	"$(INTDIR)\CMclEvent.obj" \
	"$(INTDIR)\CMclGlobal.obj" \
	"$(INTDIR)\CMclKernel.obj" \
	"$(INTDIR)\CMclMailbox.obj" \
	"$(INTDIR)\CMclMonitor.obj" \
	"$(INTDIR)\CMclMutex.obj" \
	"$(INTDIR)\CMclSemaphore.obj" \
	"$(INTDIR)\CMclSharedMemory.obj" \
	"$(INTDIR)\CMclThread.obj" \
	"$(INTDIR)\CMclWaitableCollection.obj" \
	"$(INTDIR)\expchantest.obj" \
	"$(INTDIR)\expWinConsoleDebugger.obj" \
	"$(INTDIR)\expWinConsoleDebuggerBreakPoints.obj" \
	"$(INTDIR)\expWinInit.obj" \
	"$(INTDIR)\expWinMessage.obj" \
	"$(INTDIR)\expWinSpawnChan.obj" \
	"$(INTDIR)\expWinUtils.obj" \
	"$(INTDIR)\expect.res"

"$(OUTDIR)\expect540.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug\expchantest
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : ".\MSG00409.bin" ".\expWinErr.rc" ".\expWinErr.h" "$(OUTDIR)\expect540.dll" "$(OUTDIR)\expchantest.bsc"


CLEAN :
	-@erase "$(INTDIR)\CMclAutoLock.obj"
	-@erase "$(INTDIR)\CMclAutoLock.sbr"
	-@erase "$(INTDIR)\CMclAutoPtr.obj"
	-@erase "$(INTDIR)\CMclAutoPtr.sbr"
	-@erase "$(INTDIR)\CMclCritSec.obj"
	-@erase "$(INTDIR)\CMclCritSec.sbr"
	-@erase "$(INTDIR)\CMclEvent.obj"
	-@erase "$(INTDIR)\CMclEvent.sbr"
	-@erase "$(INTDIR)\CMclGlobal.obj"
	-@erase "$(INTDIR)\CMclGlobal.sbr"
	-@erase "$(INTDIR)\CMclKernel.obj"
	-@erase "$(INTDIR)\CMclKernel.sbr"
	-@erase "$(INTDIR)\CMclMailbox.obj"
	-@erase "$(INTDIR)\CMclMailbox.sbr"
	-@erase "$(INTDIR)\CMclMonitor.obj"
	-@erase "$(INTDIR)\CMclMonitor.sbr"
	-@erase "$(INTDIR)\CMclMutex.obj"
	-@erase "$(INTDIR)\CMclMutex.sbr"
	-@erase "$(INTDIR)\CMclSemaphore.obj"
	-@erase "$(INTDIR)\CMclSemaphore.sbr"
	-@erase "$(INTDIR)\CMclSharedMemory.obj"
	-@erase "$(INTDIR)\CMclSharedMemory.sbr"
	-@erase "$(INTDIR)\CMclThread.obj"
	-@erase "$(INTDIR)\CMclThread.sbr"
	-@erase "$(INTDIR)\CMclWaitableCollection.obj"
	-@erase "$(INTDIR)\CMclWaitableCollection.sbr"
	-@erase "$(INTDIR)\expchantest.obj"
	-@erase "$(INTDIR)\expchantest.sbr"
	-@erase "$(INTDIR)\expect.res"
	-@erase "$(INTDIR)\expWinConsoleDebugger.obj"
	-@erase "$(INTDIR)\expWinConsoleDebugger.sbr"
	-@erase "$(INTDIR)\expWinConsoleDebuggerBreakPoints.obj"
	-@erase "$(INTDIR)\expWinConsoleDebuggerBreakPoints.sbr"
	-@erase "$(INTDIR)\expWinInit.obj"
	-@erase "$(INTDIR)\expWinInit.sbr"
	-@erase "$(INTDIR)\expWinMessage.obj"
	-@erase "$(INTDIR)\expWinMessage.sbr"
	-@erase "$(INTDIR)\expWinSpawnChan.obj"
	-@erase "$(INTDIR)\expWinSpawnChan.sbr"
	-@erase "$(INTDIR)\expWinUtils.obj"
	-@erase "$(INTDIR)\expWinUtils.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\expchantest.bsc"
	-@erase "$(OUTDIR)\expect540.dll"
	-@erase "$(OUTDIR)\expect540.exp"
	-@erase "$(OUTDIR)\expect540.ilk"
	-@erase "$(OUTDIR)\expect540.lib"
	-@erase "$(OUTDIR)\expect540.pdb"
	-@erase ".\expWinErr.h"
	-@erase ".\expWinErr.rc"
	-@erase ".\MSG00409.bin"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "d:\tcl_workspace\tcl_84_branch\generic" /I ".\mcl" /I "..\generic" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "USE_TCL_STUBS" /D "BUILD_exp" /D __CMCL_THROW_EXCEPTIONS__=0 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\expchantest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\expect.res" /i "..\generic" /i "d:\tcl_workspace\tcl_84_branch\generic" /i "." /d "DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\expchantest.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\CMclAutoLock.sbr" \
	"$(INTDIR)\CMclAutoPtr.sbr" \
	"$(INTDIR)\CMclCritSec.sbr" \
	"$(INTDIR)\CMclEvent.sbr" \
	"$(INTDIR)\CMclGlobal.sbr" \
	"$(INTDIR)\CMclKernel.sbr" \
	"$(INTDIR)\CMclMailbox.sbr" \
	"$(INTDIR)\CMclMonitor.sbr" \
	"$(INTDIR)\CMclMutex.sbr" \
	"$(INTDIR)\CMclSemaphore.sbr" \
	"$(INTDIR)\CMclSharedMemory.sbr" \
	"$(INTDIR)\CMclThread.sbr" \
	"$(INTDIR)\CMclWaitableCollection.sbr" \
	"$(INTDIR)\expchantest.sbr" \
	"$(INTDIR)\expWinConsoleDebugger.sbr" \
	"$(INTDIR)\expWinConsoleDebuggerBreakPoints.sbr" \
	"$(INTDIR)\expWinInit.sbr" \
	"$(INTDIR)\expWinMessage.sbr" \
	"$(INTDIR)\expWinSpawnChan.sbr" \
	"$(INTDIR)\expWinUtils.sbr"

"$(OUTDIR)\expchantest.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\expect540.pdb" /debug /machine:I386 /out:"$(OUTDIR)\expect540.dll" /implib:"$(OUTDIR)\expect540.lib" /pdbtype:sept /libpath:"d:\tcl_workspace\tcl_84_branch\win\Release" 
LINK32_OBJS= \
	"$(INTDIR)\CMclAutoLock.obj" \
	"$(INTDIR)\CMclAutoPtr.obj" \
	"$(INTDIR)\CMclCritSec.obj" \
	"$(INTDIR)\CMclEvent.obj" \
	"$(INTDIR)\CMclGlobal.obj" \
	"$(INTDIR)\CMclKernel.obj" \
	"$(INTDIR)\CMclMailbox.obj" \
	"$(INTDIR)\CMclMonitor.obj" \
	"$(INTDIR)\CMclMutex.obj" \
	"$(INTDIR)\CMclSemaphore.obj" \
	"$(INTDIR)\CMclSharedMemory.obj" \
	"$(INTDIR)\CMclThread.obj" \
	"$(INTDIR)\CMclWaitableCollection.obj" \
	"$(INTDIR)\expchantest.obj" \
	"$(INTDIR)\expWinConsoleDebugger.obj" \
	"$(INTDIR)\expWinConsoleDebuggerBreakPoints.obj" \
	"$(INTDIR)\expWinInit.obj" \
	"$(INTDIR)\expWinMessage.obj" \
	"$(INTDIR)\expWinSpawnChan.obj" \
	"$(INTDIR)\expWinUtils.obj" \
	"$(INTDIR)\expect.res"

"$(OUTDIR)\expect540.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("expchantest.dep")
!INCLUDE "expchantest.dep"
!ELSE 
!MESSAGE Warning: cannot find "expchantest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "expchantest - Win32 Release" || "$(CFG)" == "expchantest - Win32 Debug"
SOURCE=.\Mcl\CMclAutoLock.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclAutoLock.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclAutoLock.obj"	"$(INTDIR)\CMclAutoLock.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclAutoPtr.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclAutoPtr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclAutoPtr.obj"	"$(INTDIR)\CMclAutoPtr.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclCritSec.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclCritSec.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclCritSec.obj"	"$(INTDIR)\CMclCritSec.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclEvent.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclEvent.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclEvent.obj"	"$(INTDIR)\CMclEvent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclGlobal.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclGlobal.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclGlobal.obj"	"$(INTDIR)\CMclGlobal.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclKernel.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclKernel.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclKernel.obj"	"$(INTDIR)\CMclKernel.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclMailbox.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclMailbox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclMailbox.obj"	"$(INTDIR)\CMclMailbox.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclMonitor.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclMonitor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclMonitor.obj"	"$(INTDIR)\CMclMonitor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclMutex.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclMutex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclMutex.obj"	"$(INTDIR)\CMclMutex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclSemaphore.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclSemaphore.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclSemaphore.obj"	"$(INTDIR)\CMclSemaphore.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclSharedMemory.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclSharedMemory.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclSharedMemory.obj"	"$(INTDIR)\CMclSharedMemory.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclThread.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclThread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclThread.obj"	"$(INTDIR)\CMclThread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclWaitableCollection.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\CMclWaitableCollection.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\CMclWaitableCollection.obj"	"$(INTDIR)\CMclWaitableCollection.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\expchantest.c

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\expchantest.obj" : $(SOURCE) "$(INTDIR)" ".\expWinErr.h"


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\expchantest.obj"	"$(INTDIR)\expchantest.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\expect.rc

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\expect.res" : $(SOURCE) "$(INTDIR)" ".\expWinErr.rc" ".\MSG00409.bin"
	$(RSC) /l 0x409 /fo"$(INTDIR)\expect.res" /i "..\generic" /i "d:\tcl_workspace\tcl_84_branch\generic" /i "." /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\expect.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\expect.res" /i "..\generic" /i "d:\tcl_workspace\tcl_84_branch\generic" /i "." /d "DEBUG" $(SOURCE)


!ENDIF 

SOURCE=.\expWinConsoleDebugger.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\expWinConsoleDebugger.obj" : $(SOURCE) "$(INTDIR)" ".\expWinErr.h"


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\expWinConsoleDebugger.obj"	"$(INTDIR)\expWinConsoleDebugger.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\expWinConsoleDebuggerBreakPoints.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\expWinConsoleDebuggerBreakPoints.obj" : $(SOURCE) "$(INTDIR)" ".\expWinErr.h"


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\expWinConsoleDebuggerBreakPoints.obj"	"$(INTDIR)\expWinConsoleDebuggerBreakPoints.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\expWinErr.mc

!IF  "$(CFG)" == "expchantest - Win32 Release"

InputPath=.\expWinErr.mc

".\MSG00409.bin"	".\expWinErr.h"	".\expWinErr.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	mc -c -U -w $(InputPath)
<< 
	

!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"

InputPath=.\expWinErr.mc

".\MSG00409.bin"	".\expWinErr.h"	".\expWinErr.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	mc -c -U -w $(InputPath)
<< 
	

!ENDIF 

SOURCE=.\expWinInit.c

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\expWinInit.obj" : $(SOURCE) "$(INTDIR)" ".\expWinErr.h"


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\expWinInit.obj"	"$(INTDIR)\expWinInit.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\expWinMessage.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\expWinMessage.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\expWinMessage.obj"	"$(INTDIR)\expWinMessage.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\expWinSpawnChan.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\expWinSpawnChan.obj" : $(SOURCE) "$(INTDIR)" ".\expWinErr.h"


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\expWinSpawnChan.obj"	"$(INTDIR)\expWinSpawnChan.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\expWinUtils.cpp

!IF  "$(CFG)" == "expchantest - Win32 Release"


"$(INTDIR)\expWinUtils.obj" : $(SOURCE) "$(INTDIR)" ".\expWinErr.h"


!ELSEIF  "$(CFG)" == "expchantest - Win32 Debug"


"$(INTDIR)\expWinUtils.obj"	"$(INTDIR)\expWinUtils.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

