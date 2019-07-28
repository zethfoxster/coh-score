# Microsoft Developer Studio Project File - Name="ps1.0__test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ps1.0__test - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ps1.0__test.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ps1.0__test.mak" CFG="ps1.0__test - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ps1.0__test - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ps1.0__test - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ps1.0__test"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ps1.0__test - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /I "../../../include/glh" /I "../../../include/shared" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 glut32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "ps1.0__test - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "../../../include/glh" /I "../../../include/shared" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 glut32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ps1.0__test - Win32 Release"
# Name "ps1.0__test - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\_ps1.0_lexer.cpp
# End Source File
# Begin Source File

SOURCE=.\_ps1.0_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\nvparse_errors.cpp
# End Source File
# Begin Source File

SOURCE=.\ps1.0__test_main.cpp
# End Source File
# Begin Source File

SOURCE=.\ps1.0_program.cpp
# End Source File
# Begin Source File

SOURCE=..\..\shared\state_to_nvparse_text.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\_ps1.0_parser.h
# End Source File
# Begin Source File

SOURCE=.\nvparse_errors.h
# End Source File
# Begin Source File

SOURCE=.\ps1.0_program.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\shared\state_to_nvparse_text.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ps1.0_grammar.y

!IF  "$(CFG)" == "ps1.0__test - Win32 Release"

# Begin Custom Build
InputPath=.\ps1.0_grammar.y

BuildCmds= \
	bison -d -o _ps1.0_parser -p ps1.0_ ps1.0_grammar.y \
	del _ps1.0_parser.cpp \
	ren _ps1.0_parser _ps1.0_parser.cpp \
	

"_ps1.0_parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"_ps1.0_parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ps1.0__test - Win32 Debug"

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

!IF  "$(CFG)" == "ps1.0__test - Win32 Release"

# Begin Custom Build
InputPath=.\ps1.0_tokens.l

"_ps1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pps10_ -o_ps1.0_lexer.cpp ps1.0_tokens.l

# End Custom Build

!ELSEIF  "$(CFG)" == "ps1.0__test - Win32 Debug"

# Begin Custom Build
InputPath=.\ps1.0_tokens.l

"_ps1.0_lexer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -Pps10_ -o_ps1.0_lexer.cpp ps1.0_tokens.l

# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
