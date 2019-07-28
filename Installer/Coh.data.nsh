!verbose 2

; includes
  !include "${NSISDIR}\Contrib\System\System.nsh"
  ;!include "${NSISDIR}\Contrib\Modern UI\System.nsh"
!verbose 2
  !include "GetParameters.nsh"

;Include common settings
!include "COHCommon.nsh"

	OutFile "Release\CD${CD_NUMBER}\${DATA_FILE_BASENAME}${CD_NUMBER}.exe"

	ChangeUI all C:\Game\Src\Installer\Resources\tiny.exe
	
	AutoCloseWindow true
	
	
	

;--------------------------------
;Languages Include

!include "Languages.nsh"
	
!insertmacro INHERIT_LANGUAGE_FROM_WRAPPER
	

Function repositionWindow

;	!define SPI_GETWORKAREA             0x0030

	; Reposition window in the lower left
	; Create RECT struct
	System::Call "*${stRECT} .r1"
	; Find Window info for the window we're displaying
	System::Call "User32::GetWindowRect(i, i) i ($HWNDPARENT, r1) .r2"
	; Get left/top/right/bottom
	System::Call "*$1${stRECT} (.r2, .r3, .r4, .r5)"
;	MessageBox MB_OK "Got $2 $3 $4 $5"
	; Calculate width/height of our window
	IntOp $2 $4 - $2 ; $2 now contains the width
	IntOp $3 $5 - $3 ; $3 now contains the height
;	MessageBox MB_OK "Got Width = $2 Height = $3"
	
	; Determine the screen work area
	System::Call "User32::SystemParametersInfo(i, i, i, i) i (${SPI_GETWORKAREA}, 0, r1, 0) .r4" 
	; Get left/top/right/bottom
	System::Call "*$1${stRECT} (.r4, .r5, .r6, .r7)"
;	MessageBox MB_OK "Got $4 $5 $6 $7"
	
	System::Free $1

	IntOp $0 $6 - $4	; Calc width of screen
;	Messagebox MB_OK "Got Width of $0"
	; if <=8x6: x10 y155
	IntCmp $0 800 SkinnyScreen SkinnyScreen BigScreen
SkinnyScreen:
	; If this is a foreign language version, the text probably won't fit at 8x6 anyway, just move it to the corner
	StrCmp $(SPLASH_Tanker_Image) "NoText" BigScreen
	; X + 10, y - 155
	; Left side of screen + 10
	IntOp $0 $4 + 10
	
	; Bottom of screen - window - 155
	IntOp $1 $7 - $3 ; Bottom of screen - window size
	IntOp $1 $1 - 155
	Goto DoneWithChecks
	
BigScreen:
	; X + 20, Y - 20
	; Left side of screen + 20
	IntOp $0 $4 + 20

	; Bottom of screen - window - 20
	IntOp $1 $7 - $3 ; Bottom of screen - window size
	IntOp $1 $1 - 20
	Goto DoneWithChecks
	
DoneWithChecks:

	System::Call "User32::SetWindowPos(i, i, i, i, i, i, i) b ($HWNDPARENT, 0, $0, $1, 0, 0, 0x201)"

FunctionEnd


Function bgImageInit

	# the plugins dir is automatically deleted when the installer exits
	InitPluginsDir
	
	; BGImage stuff

!ifndef DISABLE_BGIMAGE

!ifdef DEBUG
	# turn return values on if in debug mode
	BgImage::SetReturn /NOUNLOAD on
!endif

	!insertmacro SPLASHIMAGEGRADIENT

	
!endif ; DISABLE_BGIMAGE

FunctionEnd ; bgImageInit




Function .onGUIEnd

	!ifndef DISABLE_BGIMAGE
		# Destroy must not have /NOUNLOAD so NSIS will be able to unload
		# and delete BgImage before it exits
		BgImage::Destroy
		# Destroy doesn't return any value
	!endif ; DISABLE_BGIMAGE

FunctionEnd



Function .onGUIInit

  Call GetParameters
  
  Pop $INSTDIR
  
  StrCmp $INSTDIR "" BadParam GoodParam
  
  BadParam:
	MessageBox MB_ICONSTOP "$(ERROR_WrongExecutable)"
	Quit
  GoodParam:

	; Make sure only one copy of the installer is running at a time.
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "CityOfHeroesData${CD_NUMBER}InstallerMutex") i .r1 ?e' 
	Pop $R0 
	StrCmp $R0 0 +3 
		MessageBox MB_OK "$(ERROR_InstallerAlreadyRunning)"
		Quit

!ifndef DEBUG
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "CityOfHeroesInstallerMutex") i .r1 ?e' 
	Pop $R0 
	StrCmp $R0 0 0 +3 
		MessageBox MB_OK "$(ERROR_WrongExecutable)"
		Quit
!endif
	Call bgImageInit
  
FunctionEnd



!include "Coh.files.nsh"  ; Misc defines needed by SINGLE_FILE installers
