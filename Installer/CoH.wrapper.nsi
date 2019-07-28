!verbose 2

;Include common settings
  !include "COHCommon.nsh"
  
;Caption "${NAME}: Extracting Setup program"

!define WRAPPED_PROGRAM "CohSetup.exe"

;--------------------------------
;Configuration

	; The file to write
	OutFile "Release\CD1\Setup.exe"

	ChangeUI all C:\Game\Src\Installer\Resources\tiny.exe
	XPStyle Off

	AutoCloseWindow true

;--------------------------------
;Languages Include

!include "Languages.nsh"


Function .onInit

!ifdef ENGLISH_ONLY

!else
	!ifdef KOREAN_ONLY
		StrCpy $LANGUAGE "1042"
	!else
		!insertmacro MUI_LANGDLL_DISPLAY
	!endif
!endif

  ; Write language string to registry for sub-installers
  WriteRegStr "${MUI_LANGDLL_REGISTRY_ROOT}" "${MUI_LANGDLL_REGISTRY_KEY}" "${MUI_LANGDLL_REGISTRY_VALUENAME} Temp" $LANGUAGE
  
FunctionEnd




;--------------------------------
;The one section
Section "copytemp"
	# the plugins dir is automatically deleted when the installer exits
	InitPluginsDir

	File "/oname=$PLUGINSDIR\${WRAPPED_PROGRAM}" "${WRAPPED_PROGRAM}"
	
	Exec '"$PLUGINSDIR\${WRAPPED_PROGRAM}" $EXEDIR'

SectionEnd