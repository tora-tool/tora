# Microsoft Developer Studio Generated NMAKE File, Based on tora.dsp
!IF "$(CFG)" == ""
CFG=tora - Win32 Debug
!MESSAGE No configuration specified. Defaulting to tora - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "tora - Win32 Release" && "$(CFG)" != "tora - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tora.mak" CFG="tora - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tora - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "tora - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "tora - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\tora.exe"


CLEAN :
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\toabout.obj"
	-@erase "$(INTDIR)\tobrowser.obj"
	-@erase "$(INTDIR)\tobrowserfilterui.obj"
	-@erase "$(INTDIR)\toconnection.obj"
	-@erase "$(INTDIR)\todebug.obj"
	-@erase "$(INTDIR)\todebugchange.obj"
	-@erase "$(INTDIR)\todebugtext.obj"
	-@erase "$(INTDIR)\todebugwatch.obj"
	-@erase "$(INTDIR)\toextract.obj"
	-@erase "$(INTDIR)\tofilesize.obj"
	-@erase "$(INTDIR)\toglobalsetting.obj"
	-@erase "$(INTDIR)\tohelp.obj"
	-@erase "$(INTDIR)\tohelpaddfileui.obj"
	-@erase "$(INTDIR)\tohelpsetupui.obj"
	-@erase "$(INTDIR)\tohighlightedtext.obj"
	-@erase "$(INTDIR)\tohtml.obj"
	-@erase "$(INTDIR)\tomain.obj"
	-@erase "$(INTDIR)\tomarkedtext.obj"
	-@erase "$(INTDIR)\tomemoeditor.obj"
	-@erase "$(INTDIR)\tonewconnection.obj"
	-@erase "$(INTDIR)\tonoblockquery.obj"
	-@erase "$(INTDIR)\tooutput.obj"
	-@erase "$(INTDIR)\toparamget.obj"
	-@erase "$(INTDIR)\topreferences.obj"
	-@erase "$(INTDIR)\toresultcols.obj"
	-@erase "$(INTDIR)\toresultconstraint.obj"
	-@erase "$(INTDIR)\toresultcontent.obj"
	-@erase "$(INTDIR)\toresultdepend.obj"
	-@erase "$(INTDIR)\toresultfield.obj"
	-@erase "$(INTDIR)\toresultindexes.obj"
	-@erase "$(INTDIR)\toresultitem.obj"
	-@erase "$(INTDIR)\toresultlock.obj"
	-@erase "$(INTDIR)\toresultlong.obj"
	-@erase "$(INTDIR)\toresultplan.obj"
	-@erase "$(INTDIR)\toresultreferences.obj"
	-@erase "$(INTDIR)\toresultstats.obj"
	-@erase "$(INTDIR)\toresultstorage.obj"
	-@erase "$(INTDIR)\toresultview.obj"
	-@erase "$(INTDIR)\torollback.obj"
	-@erase "$(INTDIR)\toscript.obj"
	-@erase "$(INTDIR)\toscriptui.obj"
	-@erase "$(INTDIR)\tosearchreplace.obj"
	-@erase "$(INTDIR)\tosearchreplaceui.obj"
	-@erase "$(INTDIR)\tosecurity.obj"
	-@erase "$(INTDIR)\tosecurityquotaui.obj"
	-@erase "$(INTDIR)\tosecurityroleui.obj"
	-@erase "$(INTDIR)\tosecurityuserui.obj"
	-@erase "$(INTDIR)\tosession.obj"
	-@erase "$(INTDIR)\tosgastatement.obj"
	-@erase "$(INTDIR)\tosgatrace.obj"
	-@erase "$(INTDIR)\tosql.obj"
	-@erase "$(INTDIR)\tosqledit.obj"
	-@erase "$(INTDIR)\tostorage.obj"
	-@erase "$(INTDIR)\tostoragedefinition.obj"
	-@erase "$(INTDIR)\tosyntaxsetup.obj"
	-@erase "$(INTDIR)\totemplate.obj"
	-@erase "$(INTDIR)\totemplateaddfileui.obj"
	-@erase "$(INTDIR)\totemplateeditui.obj"
	-@erase "$(INTDIR)\totemplateprovider.obj"
	-@erase "$(INTDIR)\totemplatesetupui.obj"
	-@erase "$(INTDIR)\tothread.obj"
	-@erase "$(INTDIR)\totool.obj"
	-@erase "$(INTDIR)\totuning.obj"
	-@erase "$(INTDIR)\toworksheet.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\tora.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /I "c:\qt\include" /I "c:\oracle\ora81\oci\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "OTL_ORA8I" /D TO_NAMESPACE="namespace std {}; using namespace std" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "NO_DEBUG" /Fp"$(INTDIR)\tora.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tora.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=oci.lib qt-mt230nc.lib qtmain.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib imm32.lib winmm.lib wsock32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\tora.pdb" /machine:I386 /out:"$(OUTDIR)\tora.exe" /libpath:"c:\Oracle\ora81\oci\lib\msvc" /libpath:"c:\qt\lib" 
LINK32_OBJS= \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\toabout.obj" \
	"$(INTDIR)\tobrowser.obj" \
	"$(INTDIR)\tobrowserfilterui.obj" \
	"$(INTDIR)\toconnection.obj" \
	"$(INTDIR)\todebug.obj" \
	"$(INTDIR)\todebugchange.obj" \
	"$(INTDIR)\todebugtext.obj" \
	"$(INTDIR)\todebugwatch.obj" \
	"$(INTDIR)\toextract.obj" \
	"$(INTDIR)\tofilesize.obj" \
	"$(INTDIR)\toglobalsetting.obj" \
	"$(INTDIR)\tohelp.obj" \
	"$(INTDIR)\tohelpaddfileui.obj" \
	"$(INTDIR)\tohelpsetupui.obj" \
	"$(INTDIR)\tohighlightedtext.obj" \
	"$(INTDIR)\tohtml.obj" \
	"$(INTDIR)\tomain.obj" \
	"$(INTDIR)\tomarkedtext.obj" \
	"$(INTDIR)\tomemoeditor.obj" \
	"$(INTDIR)\tonewconnection.obj" \
	"$(INTDIR)\tonoblockquery.obj" \
	"$(INTDIR)\tooutput.obj" \
	"$(INTDIR)\toparamget.obj" \
	"$(INTDIR)\topreferences.obj" \
	"$(INTDIR)\toresultcols.obj" \
	"$(INTDIR)\toresultconstraint.obj" \
	"$(INTDIR)\toresultcontent.obj" \
	"$(INTDIR)\toresultdepend.obj" \
	"$(INTDIR)\toresultfield.obj" \
	"$(INTDIR)\toresultindexes.obj" \
	"$(INTDIR)\toresultitem.obj" \
	"$(INTDIR)\toresultlock.obj" \
	"$(INTDIR)\toresultlong.obj" \
	"$(INTDIR)\toresultplan.obj" \
	"$(INTDIR)\toresultreferences.obj" \
	"$(INTDIR)\toresultstats.obj" \
	"$(INTDIR)\toresultstorage.obj" \
	"$(INTDIR)\toresultview.obj" \
	"$(INTDIR)\torollback.obj" \
	"$(INTDIR)\toscript.obj" \
	"$(INTDIR)\toscriptui.obj" \
	"$(INTDIR)\tosearchreplace.obj" \
	"$(INTDIR)\tosearchreplaceui.obj" \
	"$(INTDIR)\tosecurity.obj" \
	"$(INTDIR)\tosecurityquotaui.obj" \
	"$(INTDIR)\tosecurityroleui.obj" \
	"$(INTDIR)\tosecurityuserui.obj" \
	"$(INTDIR)\tosession.obj" \
	"$(INTDIR)\tosgastatement.obj" \
	"$(INTDIR)\tosgatrace.obj" \
	"$(INTDIR)\tosql.obj" \
	"$(INTDIR)\tosqledit.obj" \
	"$(INTDIR)\tostorage.obj" \
	"$(INTDIR)\tostoragedefinition.obj" \
	"$(INTDIR)\tosyntaxsetup.obj" \
	"$(INTDIR)\totemplate.obj" \
	"$(INTDIR)\totemplateaddfileui.obj" \
	"$(INTDIR)\totemplateeditui.obj" \
	"$(INTDIR)\totemplateprovider.obj" \
	"$(INTDIR)\totemplatesetupui.obj" \
	"$(INTDIR)\tothread.obj" \
	"$(INTDIR)\totool.obj" \
	"$(INTDIR)\totuning.obj" \
	"$(INTDIR)\toworksheet.obj" \
	"$(INTDIR)\utils.obj"

"$(OUTDIR)\tora.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tora - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\tora.exe"


CLEAN :
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\toabout.obj"
	-@erase "$(INTDIR)\tobrowser.obj"
	-@erase "$(INTDIR)\tobrowserfilterui.obj"
	-@erase "$(INTDIR)\toconnection.obj"
	-@erase "$(INTDIR)\todebug.obj"
	-@erase "$(INTDIR)\todebugchange.obj"
	-@erase "$(INTDIR)\todebugtext.obj"
	-@erase "$(INTDIR)\todebugwatch.obj"
	-@erase "$(INTDIR)\toextract.obj"
	-@erase "$(INTDIR)\tofilesize.obj"
	-@erase "$(INTDIR)\toglobalsetting.obj"
	-@erase "$(INTDIR)\tohelp.obj"
	-@erase "$(INTDIR)\tohelpaddfileui.obj"
	-@erase "$(INTDIR)\tohelpsetupui.obj"
	-@erase "$(INTDIR)\tohighlightedtext.obj"
	-@erase "$(INTDIR)\tohtml.obj"
	-@erase "$(INTDIR)\tomain.obj"
	-@erase "$(INTDIR)\tomarkedtext.obj"
	-@erase "$(INTDIR)\tomemoeditor.obj"
	-@erase "$(INTDIR)\tonewconnection.obj"
	-@erase "$(INTDIR)\tonoblockquery.obj"
	-@erase "$(INTDIR)\tooutput.obj"
	-@erase "$(INTDIR)\toparamget.obj"
	-@erase "$(INTDIR)\topreferences.obj"
	-@erase "$(INTDIR)\toresultcols.obj"
	-@erase "$(INTDIR)\toresultconstraint.obj"
	-@erase "$(INTDIR)\toresultcontent.obj"
	-@erase "$(INTDIR)\toresultdepend.obj"
	-@erase "$(INTDIR)\toresultfield.obj"
	-@erase "$(INTDIR)\toresultindexes.obj"
	-@erase "$(INTDIR)\toresultitem.obj"
	-@erase "$(INTDIR)\toresultlock.obj"
	-@erase "$(INTDIR)\toresultlong.obj"
	-@erase "$(INTDIR)\toresultplan.obj"
	-@erase "$(INTDIR)\toresultreferences.obj"
	-@erase "$(INTDIR)\toresultstats.obj"
	-@erase "$(INTDIR)\toresultstorage.obj"
	-@erase "$(INTDIR)\toresultview.obj"
	-@erase "$(INTDIR)\torollback.obj"
	-@erase "$(INTDIR)\toscript.obj"
	-@erase "$(INTDIR)\toscriptui.obj"
	-@erase "$(INTDIR)\tosearchreplace.obj"
	-@erase "$(INTDIR)\tosearchreplaceui.obj"
	-@erase "$(INTDIR)\tosecurity.obj"
	-@erase "$(INTDIR)\tosecurityquotaui.obj"
	-@erase "$(INTDIR)\tosecurityroleui.obj"
	-@erase "$(INTDIR)\tosecurityuserui.obj"
	-@erase "$(INTDIR)\tosession.obj"
	-@erase "$(INTDIR)\tosgastatement.obj"
	-@erase "$(INTDIR)\tosgatrace.obj"
	-@erase "$(INTDIR)\tosql.obj"
	-@erase "$(INTDIR)\tosqledit.obj"
	-@erase "$(INTDIR)\tostorage.obj"
	-@erase "$(INTDIR)\tostoragedefinition.obj"
	-@erase "$(INTDIR)\tosyntaxsetup.obj"
	-@erase "$(INTDIR)\totemplate.obj"
	-@erase "$(INTDIR)\totemplateaddfileui.obj"
	-@erase "$(INTDIR)\totemplateeditui.obj"
	-@erase "$(INTDIR)\totemplateprovider.obj"
	-@erase "$(INTDIR)\totemplatesetupui.obj"
	-@erase "$(INTDIR)\tothread.obj"
	-@erase "$(INTDIR)\totool.obj"
	-@erase "$(INTDIR)\totuning.obj"
	-@erase "$(INTDIR)\toworksheet.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\tora.exe"
	-@erase "$(OUTDIR)\tora.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /GR /GX /Zi /Od /I "c:\qt\include" /I "c:\oracle\ora81\oci\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "OTL_ORA8I" /D TO_NAMESPACE="namespace std {}; using namespace std" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "NO_DEBUG" /Fp"$(INTDIR)\tora.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tora.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=oci.lib qt-mt230nc.lib qtmain.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib imm32.lib winmm.lib wsock32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\tora.pdb" /debug /machine:I386 /out:"$(OUTDIR)\tora.exe" /pdbtype:sept /libpath:"c:\Oracle\ora81\oci\lib\msvc" /libpath:"c:\qt\lib" 
LINK32_OBJS= \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\toabout.obj" \
	"$(INTDIR)\tobrowser.obj" \
	"$(INTDIR)\tobrowserfilterui.obj" \
	"$(INTDIR)\toconnection.obj" \
	"$(INTDIR)\todebug.obj" \
	"$(INTDIR)\todebugchange.obj" \
	"$(INTDIR)\todebugtext.obj" \
	"$(INTDIR)\todebugwatch.obj" \
	"$(INTDIR)\toextract.obj" \
	"$(INTDIR)\tofilesize.obj" \
	"$(INTDIR)\toglobalsetting.obj" \
	"$(INTDIR)\tohelp.obj" \
	"$(INTDIR)\tohelpaddfileui.obj" \
	"$(INTDIR)\tohelpsetupui.obj" \
	"$(INTDIR)\tohighlightedtext.obj" \
	"$(INTDIR)\tohtml.obj" \
	"$(INTDIR)\tomain.obj" \
	"$(INTDIR)\tomarkedtext.obj" \
	"$(INTDIR)\tomemoeditor.obj" \
	"$(INTDIR)\tonewconnection.obj" \
	"$(INTDIR)\tonoblockquery.obj" \
	"$(INTDIR)\tooutput.obj" \
	"$(INTDIR)\toparamget.obj" \
	"$(INTDIR)\topreferences.obj" \
	"$(INTDIR)\toresultcols.obj" \
	"$(INTDIR)\toresultconstraint.obj" \
	"$(INTDIR)\toresultcontent.obj" \
	"$(INTDIR)\toresultdepend.obj" \
	"$(INTDIR)\toresultfield.obj" \
	"$(INTDIR)\toresultindexes.obj" \
	"$(INTDIR)\toresultitem.obj" \
	"$(INTDIR)\toresultlock.obj" \
	"$(INTDIR)\toresultlong.obj" \
	"$(INTDIR)\toresultplan.obj" \
	"$(INTDIR)\toresultreferences.obj" \
	"$(INTDIR)\toresultstats.obj" \
	"$(INTDIR)\toresultstorage.obj" \
	"$(INTDIR)\toresultview.obj" \
	"$(INTDIR)\torollback.obj" \
	"$(INTDIR)\toscript.obj" \
	"$(INTDIR)\toscriptui.obj" \
	"$(INTDIR)\tosearchreplace.obj" \
	"$(INTDIR)\tosearchreplaceui.obj" \
	"$(INTDIR)\tosecurity.obj" \
	"$(INTDIR)\tosecurityquotaui.obj" \
	"$(INTDIR)\tosecurityroleui.obj" \
	"$(INTDIR)\tosecurityuserui.obj" \
	"$(INTDIR)\tosession.obj" \
	"$(INTDIR)\tosgastatement.obj" \
	"$(INTDIR)\tosgatrace.obj" \
	"$(INTDIR)\tosql.obj" \
	"$(INTDIR)\tosqledit.obj" \
	"$(INTDIR)\tostorage.obj" \
	"$(INTDIR)\tostoragedefinition.obj" \
	"$(INTDIR)\tosyntaxsetup.obj" \
	"$(INTDIR)\totemplate.obj" \
	"$(INTDIR)\totemplateaddfileui.obj" \
	"$(INTDIR)\totemplateeditui.obj" \
	"$(INTDIR)\totemplateprovider.obj" \
	"$(INTDIR)\totemplatesetupui.obj" \
	"$(INTDIR)\tothread.obj" \
	"$(INTDIR)\totool.obj" \
	"$(INTDIR)\totuning.obj" \
	"$(INTDIR)\toworksheet.obj" \
	"$(INTDIR)\utils.obj"

"$(OUTDIR)\tora.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("tora.dep")
!INCLUDE "tora.dep"
!ELSE 
!MESSAGE Warning: cannot find "tora.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "tora - Win32 Release" || "$(CFG)" == "tora - Win32 Debug"
SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toabout.cpp

"$(INTDIR)\toabout.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tobrowser.cpp

"$(INTDIR)\tobrowser.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tobrowserfilterui.cpp

"$(INTDIR)\tobrowserfilterui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toconnection.cpp

"$(INTDIR)\toconnection.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\todebug.cpp

"$(INTDIR)\todebug.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\todebugchange.cpp

"$(INTDIR)\todebugchange.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\todebugtext.cpp

"$(INTDIR)\todebugtext.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\todebugwatch.cpp

"$(INTDIR)\todebugwatch.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toextract.cpp

"$(INTDIR)\toextract.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tofilesize.cpp

"$(INTDIR)\tofilesize.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toglobalsetting.cpp

"$(INTDIR)\toglobalsetting.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tohelp.cpp

"$(INTDIR)\tohelp.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tohelpaddfileui.cpp

"$(INTDIR)\tohelpaddfileui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tohelpsetupui.cpp

"$(INTDIR)\tohelpsetupui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tohighlightedtext.cpp

"$(INTDIR)\tohighlightedtext.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tohtml.cpp

"$(INTDIR)\tohtml.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tomain.cpp

"$(INTDIR)\tomain.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tomarkedtext.cpp

"$(INTDIR)\tomarkedtext.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tomemoeditor.cpp

"$(INTDIR)\tomemoeditor.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tonewconnection.cpp

"$(INTDIR)\tonewconnection.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tonoblockquery.cpp

"$(INTDIR)\tonoblockquery.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tooutput.cpp

"$(INTDIR)\tooutput.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toparamget.cpp

"$(INTDIR)\toparamget.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\topreferences.cpp

"$(INTDIR)\topreferences.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultcols.cpp

"$(INTDIR)\toresultcols.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultconstraint.cpp

"$(INTDIR)\toresultconstraint.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultcontent.cpp

"$(INTDIR)\toresultcontent.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultdepend.cpp

"$(INTDIR)\toresultdepend.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultfield.cpp

"$(INTDIR)\toresultfield.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultindexes.cpp

"$(INTDIR)\toresultindexes.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultitem.cpp

"$(INTDIR)\toresultitem.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultlock.cpp

"$(INTDIR)\toresultlock.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultlong.cpp

"$(INTDIR)\toresultlong.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultplan.cpp

"$(INTDIR)\toresultplan.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultreferences.cpp

"$(INTDIR)\toresultreferences.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultstats.cpp

"$(INTDIR)\toresultstats.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultstorage.cpp

"$(INTDIR)\toresultstorage.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toresultview.cpp

"$(INTDIR)\toresultview.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\torollback.cpp

"$(INTDIR)\torollback.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toscript.cpp

"$(INTDIR)\toscript.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toscriptui.cpp

"$(INTDIR)\toscriptui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosearchreplace.cpp

"$(INTDIR)\tosearchreplace.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosearchreplaceui.cpp

"$(INTDIR)\tosearchreplaceui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosecurity.cpp

"$(INTDIR)\tosecurity.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosecurityquotaui.cpp

"$(INTDIR)\tosecurityquotaui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosecurityroleui.cpp

"$(INTDIR)\tosecurityroleui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosecurityuserui.cpp

"$(INTDIR)\tosecurityuserui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosession.cpp

"$(INTDIR)\tosession.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosgastatement.cpp

"$(INTDIR)\tosgastatement.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosgatrace.cpp

"$(INTDIR)\tosgatrace.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosql.cpp

"$(INTDIR)\tosql.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosqledit.cpp

"$(INTDIR)\tosqledit.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tostorage.cpp

"$(INTDIR)\tostorage.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tostoragedefinition.cpp

"$(INTDIR)\tostoragedefinition.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tosyntaxsetup.cpp

"$(INTDIR)\tosyntaxsetup.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\totemplate.cpp

"$(INTDIR)\totemplate.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\totemplateaddfileui.cpp

"$(INTDIR)\totemplateaddfileui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\totemplateeditui.cpp

"$(INTDIR)\totemplateeditui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\totemplateprovider.cpp

"$(INTDIR)\totemplateprovider.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\totemplatesetupui.cpp

"$(INTDIR)\totemplatesetupui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tothread.cpp

"$(INTDIR)\tothread.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\totool.cpp

"$(INTDIR)\totool.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\totuning.cpp

"$(INTDIR)\totuning.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toworksheet.cpp

"$(INTDIR)\toworksheet.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\utils.cpp

"$(INTDIR)\utils.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

