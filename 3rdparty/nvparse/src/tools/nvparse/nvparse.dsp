# Microsoft Developer Studio Project File - Name="nvparse" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nvparse - Win32 Hybrid
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nvparse.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nvparse.mak" CFG="nvparse - Win32 Hybrid"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nvparse - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nvparse - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "nvparse - Win32 Debug Multithreaded" (based on "Win32 (x86) Static Library")
!MESSAGE "nvparse - Win32 Release Multithreaded" (based on "Win32 (x86) Static Library")
!MESSAGE "nvparse - Win32 Debug Multithreaded DLL" (based on "Win32 (x86) Static Library")
!MESSAGE "nvparse - Win32 Release Multithreaded DLL" (based on "Win32 (x86) Static Library")
!MESSAGE "nvparse - Win32 Hybrid" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ceveritt-98"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nvparse - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Custom Build
InputPath=.\Release\nvparse.lib
SOURCE="$(InputPath)"

"..\..\..\lib\Release\nvparse.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy .\Release\nvparse.lib ..\..\..\lib\Release

# End Custom Build
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy .\Release\nvparse.lib ..\..\..\lib\Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy .\Debug\nvparse.lib ..\..\..\lib\Debug
# End Special Build Tool

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nvparse___Win32_Debug_Multithreaded"
# PROP BASE Intermediate_Dir "nvparse___Win32_Debug_Multithreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "nvparse___Win32_Debug_Multithreaded"
# PROP Intermediate_Dir "nvparse___Win32_Debug_Multithreaded"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\nvparse_mt.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy .\Debug\nvparse_mt.lib ..\..\..\lib\Debug\nvparse_mt.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nvparse___Win32_Release_Multithreaded"
# PROP BASE Intermediate_Dir "nvparse___Win32_Release_Multithreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "nvparse___Win32_Release_Multithreaded"
# PROP Intermediate_Dir "nvparse___Win32_Release_Multithreaded"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\nvparse_mt.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Performing Post-build copying on $(InputPath)
PostBuild_Cmds=copy .\Release\nvparse_mt.lib ..\..\..\lib\Release\nvparse_mt.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nvparse___Win32_Debug_Multithreaded_DLL"
# PROP BASE Intermediate_Dir "nvparse___Win32_Debug_Multithreaded_DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "nvparse___Win32_Debug_Multithreaded_DLL"
# PROP Intermediate_Dir "nvparse___Win32_Debug_Multithreaded_DLL"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug\nvparse_mt.lib"
# ADD LIB32 /nologo /out:"Debug\nvparse_mtdll.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy .\Debug\nvparse_mtdll.lib ..\..\..\lib\Debug\nvparse_mtdll.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nvparse___Win32_Release_Multithreaded_DLL"
# PROP BASE Intermediate_Dir "nvparse___Win32_Release_Multithreaded_DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "nvparse___Win32_Release_Multithreaded_DLL"
# PROP Intermediate_Dir "nvparse___Win32_Release_Multithreaded_DLL"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Release\nvparse_mt.lib"
# ADD LIB32 /nologo /out:"Release\nvparse_mtdll.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Performing Post-build copying on $(InputPath)
PostBuild_Cmds=copy .\Release\nvparse_mtdll.lib ..\..\..\lib\Release\nvparse_mtdll.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "nvparse - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nvparse___Win32_Hybrid"
# PROP BASE Intermediate_Dir "nvparse___Win32_Hybrid"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "nvparse___Win32_Hybrid"
# PROP Intermediate_Dir "nvparse___Win32_Hybrid"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "." /I "..\..\..\include\glh" /I "..\..\..\include\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug\nvparse_mtdll.lib"
# ADD LIB32 /nologo /out:"debug\nvparse_hybrid.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy .\Debug\nvparse_hybrid.lib ..\..\..\lib\Debug\nvparse_hybrid.lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "nvparse - Win32 Release"
# Name "nvparse - Win32 Debug"
# Name "nvparse - Win32 Debug Multithreaded"
# Name "nvparse - Win32 Release Multithreaded"
# Name "nvparse - Win32 Debug Multithreaded DLL"
# Name "nvparse - Win32 Release Multithreaded DLL"
# Name "nvparse - Win32 Hybrid"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\_ps1.0_lexer.cpp
# End Source File
# Begin Source File

SOURCE=.\_ps1.0_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\_rc1.0_lexer.cpp
# End Source File
# Begin Source File

SOURCE=.\_rc1.0_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\_ts1.0_lexer.cpp
# End Source File
# Begin Source File

SOURCE=.\_ts1.0_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\_vs1.0_lexer.cpp
# End Source File
# Begin Source File

SOURCE=.\_vs1.0_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\nvparse.cpp
# End Source File
# Begin Source File

SOURCE=.\nvparse_errors.cpp
# End Source File
# Begin Source File

SOURCE=.\ps1.0_program.cpp
# End Source File
# Begin Source File

SOURCE=.\rc1.0_combiners.cpp
# End Source File
# Begin Source File

SOURCE=.\rc1.0_final.cpp
# End Source File
# Begin Source File

SOURCE=.\rc1.0_general.cpp
# End Source File
# Begin Source File

SOURCE=.\ts1.0_inst.cpp
# End Source File
# Begin Source File

SOURCE=.\ts1.0_inst_list.cpp
# End Source File
# Begin Source File

SOURCE=.\vcp1.0_impl.cpp
# End Source File
# Begin Source File

SOURCE=.\vp1.0_impl.cpp
# End Source File
# Begin Source File

SOURCE=.\vs1.0_inst.cpp
# End Source File
# Begin Source File

SOURCE=.\vs1.0_inst_list.cpp
# End Source File
# Begin Source File

SOURCE=.\vsp1.0_impl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\_ps1.0_parser.h
# End Source File
# Begin Source File

SOURCE=.\_rc1.0_parser.h
# End Source File
# Begin Source File

SOURCE=.\_ts1.0_parser.h
# End Source File
# Begin Source File

SOURCE=.\_vs1.0_parser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\shared\nvparse.h
# End Source File
# Begin Source File

SOURCE=.\nvparse_errors.h
# End Source File
# Begin Source File

SOURCE=.\nvparse_externs.h
# End Source File
# Begin Source File

SOURCE=.\ps1.0_program.h
# End Source File
# Begin Source File

SOURCE=.\rc1.0_combiners.h
# End Source File
# Begin Source File

SOURCE=.\rc1.0_final.h
# End Source File
# Begin Source File

SOURCE=.\rc1.0_general.h
# End Source File
# Begin Source File

SOURCE=.\rc1.0_register.h
# End Source File
# Begin Source File

SOURCE=.\ts1.0_inst.h
# End Source File
# Begin Source File

SOURCE=.\ts1.0_inst_list.h
# End Source File
# Begin Source File

SOURCE=.\vs1.0_inst.h
# End Source File
# Begin Source File

SOURCE=.\vs1.0_inst_list.h
# End Source File
# End Group
# Begin Group "parsers"

# PROP Default_Filter "l;y"
# Begin Source File

SOURCE=.\ps1.0_grammar.y

!IF  "$(CFG)" == "nvparse - Win32 Release"

# Begin Custom Build
InputPath=.\ps1.0_grammar.y

BuildCmds= \
	bison -d -o _ps1.0_parser -p ps10_ ps1.0_grammar.y \
	del _ps1.0_parser.cpp \
	ren _ps1.0_parser _ps1.0_parser.cpp \
	

"_ps1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ps1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug"

# Begin Custom Build
InputPath=.\ps1.0_grammar.y

BuildCmds= \
	bison -d -o _ps1.0_parser -p ps10_ ps1.0_grammar.y \
	del _ps1.0_parser.cpp \
	ren _ps1.0_parser _ps1.0_parser.cpp \
	

"_ps1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ps1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded"

# Begin Custom Build
InputPath=.\ps1.0_grammar.y

BuildCmds= \
	bison -d -o _ps1.0_parser -p ps10_ ps1.0_grammar.y \
	del _ps1.0_parser.cpp \
	ren _ps1.0_parser _ps1.0_parser.cpp \
	

"_ps1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ps1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded"

# Begin Custom Build
InputPath=.\ps1.0_grammar.y

BuildCmds= \
	bison -d -o _ps1.0_parser -p ps10_ ps1.0_grammar.y \
	del _ps1.0_parser.cpp \
	ren _ps1.0_parser _ps1.0_parser.cpp \
	

"_ps1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ps1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded DLL"

# Begin Custom Build
InputPath=.\ps1.0_grammar.y

BuildCmds= \
	bison -d -o _ps1.0_parser -p ps10_ ps1.0_grammar.y \
	del _ps1.0_parser.cpp \
	ren _ps1.0_parser _ps1.0_parser.cpp \
	

"_ps1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ps1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded DLL"

# Begin Custom Build
InputPath=.\ps1.0_grammar.y

BuildCmds= \
	bison -d -o _ps1.0_parser -p ps10_ ps1.0_grammar.y \
	del _ps1.0_parser.cpp \
	ren _ps1.0_parser _ps1.0_parser.cpp \
	

"_ps1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ps1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Hybrid"

# Begin Custom Build
InputPath=.\ps1.0_grammar.y

BuildCmds= \
	bison -d -o _ps1.0_parser -p ps10_ ps1.0_grammar.y \
	del _ps1.0_parser.cpp \
	ren _ps1.0_parser _ps1.0_parser.cpp \
	

"_ps1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ps1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps1.0_tokens.l

!IF  "$(CFG)" == "nvparse - Win32 Release"

# Begin Custom Build
InputPath=.\ps1.0_tokens.l

"_ps1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pps10_ -o_ps1.0_lexer.cpp ps1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug"

# Begin Custom Build
InputPath=.\ps1.0_tokens.l

"_ps1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pps10_ -o_ps1.0_lexer.cpp ps1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded"

# Begin Custom Build
InputPath=.\ps1.0_tokens.l

"_ps1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pps10_ -o_ps1.0_lexer.cpp ps1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded"

# Begin Custom Build
InputPath=.\ps1.0_tokens.l

"_ps1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pps10_ -o_ps1.0_lexer.cpp ps1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded DLL"

# Begin Custom Build
InputPath=.\ps1.0_tokens.l

"_ps1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pps10_ -o_ps1.0_lexer.cpp ps1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded DLL"

# Begin Custom Build
InputPath=.\ps1.0_tokens.l

"_ps1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pps10_ -o_ps1.0_lexer.cpp ps1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Hybrid"

# Begin Custom Build
InputPath=.\ps1.0_tokens.l

"_ps1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pps10_ -o_ps1.0_lexer.cpp ps1.0_tokens.l

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rc1.0_grammar.y

!IF  "$(CFG)" == "nvparse - Win32 Release"

# Begin Custom Build
InputPath=.\rc1.0_grammar.y

BuildCmds= \
	bison -d -o _rc1.0_parser -p rc10_ rc1.0_grammar.y \
	del _rc1.0_parser.cpp \
	ren _rc1.0_parser _rc1.0_parser.cpp \
	

"_rc1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_rc1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug"

# Begin Custom Build
InputPath=.\rc1.0_grammar.y

BuildCmds= \
	bison -d -o _rc1.0_parser -p rc10_ rc1.0_grammar.y \
	del _rc1.0_parser.cpp \
	ren _rc1.0_parser _rc1.0_parser.cpp \
	

"_rc1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_rc1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded"

# Begin Custom Build
InputPath=.\rc1.0_grammar.y

BuildCmds= \
	bison -d -o _rc1.0_parser -p rc10_ rc1.0_grammar.y \
	del _rc1.0_parser.cpp \
	ren _rc1.0_parser _rc1.0_parser.cpp \
	

"_rc1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_rc1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded"

# Begin Custom Build
InputPath=.\rc1.0_grammar.y

BuildCmds= \
	bison -d -o _rc1.0_parser -p rc10_ rc1.0_grammar.y \
	del _rc1.0_parser.cpp \
	ren _rc1.0_parser _rc1.0_parser.cpp \
	

"_rc1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_rc1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded DLL"

# Begin Custom Build
InputPath=.\rc1.0_grammar.y

BuildCmds= \
	bison -d -o _rc1.0_parser -p rc10_ rc1.0_grammar.y \
	del _rc1.0_parser.cpp \
	ren _rc1.0_parser _rc1.0_parser.cpp \
	

"_rc1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_rc1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded DLL"

# Begin Custom Build
InputPath=.\rc1.0_grammar.y

BuildCmds= \
	bison -d -o _rc1.0_parser -p rc10_ rc1.0_grammar.y \
	del _rc1.0_parser.cpp \
	ren _rc1.0_parser _rc1.0_parser.cpp \
	

"_rc1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_rc1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Hybrid"

# Begin Custom Build
InputPath=.\rc1.0_grammar.y

BuildCmds= \
	bison -d -o _rc1.0_parser -p rc10_ rc1.0_grammar.y \
	del _rc1.0_parser.cpp \
	ren _rc1.0_parser _rc1.0_parser.cpp \
	

"_rc1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_rc1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rc1.0_tokens.l

!IF  "$(CFG)" == "nvparse - Win32 Release"

# Begin Custom Build
InputPath=.\rc1.0_tokens.l

"_rc1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Prc10_ -o_rc1.0_lexer.cpp rc1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug"

# Begin Custom Build
InputPath=.\rc1.0_tokens.l

"_rc1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Prc10_ -o_rc1.0_lexer.cpp rc1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded"

# Begin Custom Build
InputPath=.\rc1.0_tokens.l

"_rc1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Prc10_ -o_rc1.0_lexer.cpp rc1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded"

# Begin Custom Build
InputPath=.\rc1.0_tokens.l

"_rc1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Prc10_ -o_rc1.0_lexer.cpp rc1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded DLL"

# Begin Custom Build
InputPath=.\rc1.0_tokens.l

"_rc1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Prc10_ -o_rc1.0_lexer.cpp rc1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded DLL"

# Begin Custom Build
InputPath=.\rc1.0_tokens.l

"_rc1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Prc10_ -o_rc1.0_lexer.cpp rc1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Hybrid"

# Begin Custom Build
InputPath=.\rc1.0_tokens.l

"_rc1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Prc10_ -o_rc1.0_lexer.cpp rc1.0_tokens.l

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts1.0_grammar.y

!IF  "$(CFG)" == "nvparse - Win32 Release"

# Begin Custom Build
InputPath=.\ts1.0_grammar.y

BuildCmds= \
	bison -d -o _ts1.0_parser -p ts10_ ts1.0_grammar.y \
	del _ts1.0_parser.cpp \
	ren  _ts1.0_parser _ts1.0_parser.cpp \
	

"_ts1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ts1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug"

# Begin Custom Build
InputPath=.\ts1.0_grammar.y

BuildCmds= \
	bison -d -o _ts1.0_parser -p ts10_ ts1.0_grammar.y \
	del _ts1.0_parser.cpp \
	ren  _ts1.0_parser _ts1.0_parser.cpp \
	

"_ts1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ts1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded"

# Begin Custom Build
InputPath=.\ts1.0_grammar.y

BuildCmds= \
	bison -d -o _ts1.0_parser -p ts10_ ts1.0_grammar.y \
	del _ts1.0_parser.cpp \
	ren  _ts1.0_parser _ts1.0_parser.cpp \
	

"_ts1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ts1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded"

# Begin Custom Build
InputPath=.\ts1.0_grammar.y

BuildCmds= \
	bison -d -o _ts1.0_parser -p ts10_ ts1.0_grammar.y \
	del _ts1.0_parser.cpp \
	ren  _ts1.0_parser _ts1.0_parser.cpp \
	

"_ts1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ts1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded DLL"

# Begin Custom Build
InputPath=.\ts1.0_grammar.y

BuildCmds= \
	bison -d -o _ts1.0_parser -p ts10_ ts1.0_grammar.y \
	del _ts1.0_parser.cpp \
	ren  _ts1.0_parser _ts1.0_parser.cpp \
	

"_ts1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ts1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded DLL"

# Begin Custom Build
InputPath=.\ts1.0_grammar.y

BuildCmds= \
	bison -d -o _ts1.0_parser -p ts10_ ts1.0_grammar.y \
	del _ts1.0_parser.cpp \
	ren  _ts1.0_parser _ts1.0_parser.cpp \
	

"_ts1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ts1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Hybrid"

# Begin Custom Build
InputPath=.\ts1.0_grammar.y

BuildCmds= \
	bison -d -o _ts1.0_parser -p ts10_ ts1.0_grammar.y \
	del _ts1.0_parser.cpp \
	ren  _ts1.0_parser _ts1.0_parser.cpp \
	

"_ts1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ts1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts1.0_tokens.l

!IF  "$(CFG)" == "nvparse - Win32 Release"

# Begin Custom Build
InputPath=.\ts1.0_tokens.l

"_ts1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pts10_ -o_ts1.0_lexer.cpp ts1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug"

# Begin Custom Build
InputPath=.\ts1.0_tokens.l

"_ts1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pts10_ -o_ts1.0_lexer.cpp ts1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded"

# Begin Custom Build
InputPath=.\ts1.0_tokens.l

"_ts1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pts10_ -o_ts1.0_lexer.cpp ts1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded"

# Begin Custom Build
InputPath=.\ts1.0_tokens.l

"_ts1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pts10_ -o_ts1.0_lexer.cpp ts1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded DLL"

# Begin Custom Build
InputPath=.\ts1.0_tokens.l

"_ts1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pts10_ -o_ts1.0_lexer.cpp ts1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded DLL"

# Begin Custom Build
InputPath=.\ts1.0_tokens.l

"_ts1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pts10_ -o_ts1.0_lexer.cpp ts1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Hybrid"

# Begin Custom Build
InputPath=.\ts1.0_tokens.l

"_ts1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pts10_ -o_ts1.0_lexer.cpp ts1.0_tokens.l

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vs1.0_grammar.y

!IF  "$(CFG)" == "nvparse - Win32 Release"

# Begin Custom Build
InputPath=.\vs1.0_grammar.y

BuildCmds= \
	bison -d -o _vs1.0_parser -p vs10_ vs1.0_grammar.y \
	del _vs1.0_parser.cpp \
	ren  _vs1.0_parser _vs1.0_parser.cpp \
	

"_vs1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_vs1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug"

# Begin Custom Build
InputPath=.\vs1.0_grammar.y

BuildCmds= \
	bison -d -o _vs1.0_parser -p vs10_ vs1.0_grammar.y \
	del _vs1.0_parser.cpp \
	ren  _vs1.0_parser _vs1.0_parser.cpp \
	

"_vs1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_vs1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded"

# Begin Custom Build
InputPath=.\vs1.0_grammar.y

BuildCmds= \
	bison -d -o _vs1.0_parser -p vs10_ vs1.0_grammar.y \
	del _vs1.0_parser.cpp \
	ren  _vs1.0_parser _vs1.0_parser.cpp \
	

"_vs1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_vs1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded"

# Begin Custom Build
InputPath=.\vs1.0_grammar.y

BuildCmds= \
	bison -d -o _vs1.0_parser -p vs10_ vs1.0_grammar.y \
	del _vs1.0_parser.cpp \
	ren  _vs1.0_parser _vs1.0_parser.cpp \
	

"_vs1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_vs1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded DLL"

# Begin Custom Build
InputPath=.\vs1.0_grammar.y

BuildCmds= \
	bison -d -o _vs1.0_parser -p vs10_ vs1.0_grammar.y \
	del _vs1.0_parser.cpp \
	ren  _vs1.0_parser _vs1.0_parser.cpp \
	

"_vs1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_vs1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded DLL"

# Begin Custom Build
InputPath=.\vs1.0_grammar.y

BuildCmds= \
	bison -d -o _vs1.0_parser -p vs10_ vs1.0_grammar.y \
	del _vs1.0_parser.cpp \
	ren  _vs1.0_parser _vs1.0_parser.cpp \
	

"_vs1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_vs1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Hybrid"

# Begin Custom Build
InputPath=.\vs1.0_grammar.y

BuildCmds= \
	bison -d -o _vs1.0_parser -p vs10_ vs1.0_grammar.y \
	del _vs1.0_parser.cpp \
	ren  _vs1.0_parser _vs1.0_parser.cpp \
	

"_vs1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_vs1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vs1.0_tokens.l

!IF  "$(CFG)" == "nvparse - Win32 Release"

# Begin Custom Build
InputPath=.\vs1.0_tokens.l

"_vs1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pvs10_ -o_vs1.0_lexer.cpp vs1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug"

# Begin Custom Build
InputPath=.\vs1.0_tokens.l

"_vs1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pvs10_ -o_vs1.0_lexer.cpp vs1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded"

# Begin Custom Build
InputPath=.\vs1.0_tokens.l

"_vs1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pvs10_ -o_vs1.0_lexer.cpp vs1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded"

# Begin Custom Build
InputPath=.\vs1.0_tokens.l

"_vs1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pvs10_ -o_vs1.0_lexer.cpp vs1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Debug Multithreaded DLL"

# Begin Custom Build
InputPath=.\vs1.0_tokens.l

"_vs1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pvs10_ -o_vs1.0_lexer.cpp vs1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Release Multithreaded DLL"

# Begin Custom Build
InputPath=.\vs1.0_tokens.l

"_vs1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pvs10_ -o_vs1.0_lexer.cpp vs1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "nvparse - Win32 Hybrid"

# Begin Custom Build
InputPath=.\vs1.0_tokens.l

"_vs1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pvs10_ -o_vs1.0_lexer.cpp vs1.0_tokens.l

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
