# Microsoft Developer Studio Generated NMAKE File, Based on injector.dsp
!IF "$(CFG)" == ""
CFG=injector - Win32 Debug
!MESSAGE No configuration specified. Defaulting to injector - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "injector - Win32 Release" && "$(CFG)" != "injector - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "injector - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release\injector
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\injector.dll"


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
	-@erase "$(INTDIR)\expWinInjectorMain.obj"
	-@erase "$(INTDIR)\inject.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\injector.dll"
	-@erase "$(OUTDIR)\injector.exp"
	-@erase "$(OUTDIR)\injector.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\mcl" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D __CMCL_THROW_EXCEPTIONS__=0 /Fp"$(INTDIR)\injector.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\inject.res" /i "..\generic" /i "d:\tcl_workspace\tcl_84_branch\generic" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\injector.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib /nologo /subsystem:console /dll /incremental:no /pdb:"$(OUTDIR)\injector.pdb" /machine:I386 /out:"$(OUTDIR)\injector.dll" /implib:"$(OUTDIR)\injector.lib" /release /opt:nowin98 
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
	"$(INTDIR)\expWinInjectorMain.obj" \
	"$(INTDIR)\inject.res"

"$(OUTDIR)\injector.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "injector - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug\injector
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\injector.dll" "$(OUTDIR)\injector.bsc"


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
	-@erase "$(INTDIR)\expWinInjectorMain.obj"
	-@erase "$(INTDIR)\expWinInjectorMain.sbr"
	-@erase "$(INTDIR)\inject.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\injector.bsc"
	-@erase "$(OUTDIR)\injector.dll"
	-@erase "$(OUTDIR)\injector.exp"
	-@erase "$(OUTDIR)\injector.ilk"
	-@erase "$(OUTDIR)\injector.lib"
	-@erase "$(OUTDIR)\injector.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /ZI /Od /I ".\mcl" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D __CMCL_THROW_EXCEPTIONS__=0 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\injector.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\inject.res" /i "..\generic" /i "d:\tcl_workspace\tcl_84_branch\generic" /d "DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\injector.bsc" 
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
	"$(INTDIR)\expWinInjectorMain.sbr"

"$(OUTDIR)\injector.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\injector.pdb" /debug /machine:I386 /out:"$(OUTDIR)\injector.dll" /implib:"$(OUTDIR)\injector.lib" /pdbtype:sept 
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
	"$(INTDIR)\expWinInjectorMain.obj" \
	"$(INTDIR)\inject.res"

"$(OUTDIR)\injector.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("injector.dep")
!INCLUDE "injector.dep"
!ELSE 
!MESSAGE Warning: cannot find "injector.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "injector - Win32 Release" || "$(CFG)" == "injector - Win32 Debug"
SOURCE=.\Mcl\CMclAutoLock.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclAutoLock.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclAutoLock.obj"	"$(INTDIR)\CMclAutoLock.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclAutoPtr.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclAutoPtr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclAutoPtr.obj"	"$(INTDIR)\CMclAutoPtr.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclCritSec.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclCritSec.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclCritSec.obj"	"$(INTDIR)\CMclCritSec.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclEvent.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclEvent.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclEvent.obj"	"$(INTDIR)\CMclEvent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclGlobal.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclGlobal.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclGlobal.obj"	"$(INTDIR)\CMclGlobal.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclKernel.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclKernel.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclKernel.obj"	"$(INTDIR)\CMclKernel.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclMailbox.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclMailbox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclMailbox.obj"	"$(INTDIR)\CMclMailbox.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclMonitor.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclMonitor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclMonitor.obj"	"$(INTDIR)\CMclMonitor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclMutex.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclMutex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclMutex.obj"	"$(INTDIR)\CMclMutex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclSemaphore.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclSemaphore.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclSemaphore.obj"	"$(INTDIR)\CMclSemaphore.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclSharedMemory.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclSharedMemory.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclSharedMemory.obj"	"$(INTDIR)\CMclSharedMemory.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclThread.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclThread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclThread.obj"	"$(INTDIR)\CMclThread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Mcl\CMclWaitableCollection.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\CMclWaitableCollection.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\CMclWaitableCollection.obj"	"$(INTDIR)\CMclWaitableCollection.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\expWinInjectorMain.cpp

!IF  "$(CFG)" == "injector - Win32 Release"


"$(INTDIR)\expWinInjectorMain.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "injector - Win32 Debug"


"$(INTDIR)\expWinInjectorMain.obj"	"$(INTDIR)\expWinInjectorMain.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\inject.rc

"$(INTDIR)\inject.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

