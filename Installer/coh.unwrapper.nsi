!verbose 2
!include "GetParameters.nsh"

;Include common settings
  !include "COHCommon.nsh"
  
;--------------------------------
;Languages Include

!include "Languages.nsh"
	
!insertmacro INHERIT_LANGUAGE_FROM_WRAPPER
	
!define WRAPPED_PROGRAM "CohSetup.exe"

;--------------------------------
;Configuration

	; The file to write
	OutFile "Release\CD1\FinishSetup.exe"

	ChangeUI all C:\Game\Src\Installer\Resources\tiny.exe
	XPStyle Off

	AutoCloseWindow true
	SilentInstall silent
	
;--------------------------------
;The one section
Section "deletetemp"
	SectionSetFlags 0 SF_SELECTED
	
	Call GetParameters
	Pop $0

	StrCmp $0 "/DONOTHING" DoneNothing

	StrCmp $0 "" BadParam

	IfFileExists "$0\${WRAPPED_PROGRAM}" GoodParam

	BadParam:
		MessageBox MB_ICONSTOP "$(ERROR_WrongExecutable)"
		Quit
	GoodParam:
	
	IntOp $1 0 + 0
	Begin:
		Delete "$0\${WRAPPED_PROGRAM}"
		IntOp $1 $1 + 1
		IfErrors 0 Finished
		IntCmp $1 100 Finished ; Cancel if it can't delete for 10 seconds
		Sleep 100
		Goto Begin
	
	Finished:
	    RMDir "$0"
	    
	DoneNothing:

SectionEnd