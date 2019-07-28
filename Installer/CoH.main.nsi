;SetCompressor lzma
; includes
!verbose 3
!include "${NSISDIR}\Contrib\System\System.nsh"
; !include "${NSISDIR}\Contrib\Modern UI\System.nsh"
!verbose 3

;--------------------------------
;Include common settings
	!include "COHCommon.nsh"

!ifdef MAKE_UPDATER_ONLY
!else ifndef SINGLE_FILE
	!include "FindFileOnRoot.nsh"
	!include "GetParameters.nsh"
!else
	!include "Coh.files.nsh"
!endif
  
  
;--------------------------------
; MUI override hooks
!define MUI_CUSTOMFUNCTION_GUIINIT onGUIInit_Custom
;!define MUI_CUSTOMFUNCTION_ABORT .onGUIEnd
!ifdef SINGLE_FILE
	!define MUI_UI "C:\game\src\Installer\Resources\UIs\modernsmall.exe"
	!define MUI_UI_HEADERIMAGE "C:\game\src\Installer\Resources\UIs\modernsmall_headerbmp.exe"
	!define MUI_UI_COMPONENTSPAGE_SMALLDESC "C:\game\src\Installer\Resources\UIs\modernsmall_smalldesc.exe"
!endif

; Header 150x57
!ifdef COV_INSTALLER
	!define MUI_HEADERIMAGE_BITMAP "C:\Game\src\Installer\V_Resources\Vheader.bmp"
!else
	!ifdef KOREAN_ONLY
		!define MUI_HEADERIMAGE_BITMAP "C:\Game\src\Installer\Resources\header_Korean.bmp"
	!else
		!define MUI_HEADERIMAGE_BITMAP "C:\Game\src\Installer\Resources\header.bmp"
	!endif
!endif
!define MUI_COMPONENTSPAGE_SMALLDESC

; Finish page
!define MUI_FINISHPAGE_TITLE $(DESC_FinishPageTitle)
!define MUI_FINISHPAGE_TEXT $(DESC_FinishPageText)

!ifdef GR_INSTALLER
	!define MUI_FINISHPAGE_RUN $INSTDIR\${UPDATER_OUTPUT_NAME}
	!ifdef GR_BETA_INSTALLER
		!ifdef ENGLISH_ONLY
			!define MUI_FINISHPAGE_RUN_PARAMETERS "-uiskin 2 -beta"
		!else
			!define MUI_FINISHPAGE_RUN_PARAMETERS "-uiskin 2 -test"
		!endif
	!else
		!define MUI_FINISHPAGE_RUN_PARAMETERS "-uiskin 2"
	!endif
!else ifdef COV_INSTALLER
	!ifdef BETA_INSTALLER
		!define MUI_FINISHPAGE_RUN $INSTDIR\${UPDATER_OUTPUT_NAME}
	!else
		!define MUI_FINISHPAGE_RUN $INSTDIR\${COV_UPDATER_OUTPUT_NAME}
	!endif
!else
	!define MUI_FINISHPAGE_RUN $INSTDIR\${UPDATER_OUTPUT_NAME}
!endif

!define MUI_FINISHPAGE_RUN_TEXT $(DESC_FinishPageRunText)

!ifndef MAKE_UPDATER_ONLY
!ifndef KOREAN_ONLY
!ifndef GR_BETA_INSTALLER
	!ifndef BETA_INSTALLER
		!define MUI_FINISHPAGE_SHOWREADME $(DESC_ShowReadmeFile)
	!endif
!endif
!endif
!endif

; recommended size: 164x314 pixels
; Default: ${NSISDIR}\Contrib\Graphics\Wizard\win.bmp
!ifdef COV_INSTALLER
	!define MUI_WELCOMEFINISHPAGE_BITMAP C:\Game\src\installer\V_Resources\Vsidebar.bmp
!else
	!ifdef KOREAN_ONLY
		!define MUI_WELCOMEFINISHPAGE_BITMAP C:\Game\src\installer\resources\Sidebar_Korean.bmp
	!else
		!define MUI_WELCOMEFINISHPAGE_BITMAP C:\Game\src\installer\resources\sidebar.bmp
	!endif
!endif

!define MUI_UNCONFIRMPAGE_TEXT_TOP $(DESC_UnConfirmPageText)
	UninstallCaption $(DESC_UnConfirmPageCaption)


;--------------------------------
;Include Modern UI
	!include "MUI.nsh"


;--------------------------------
;Configuration

!ifdef SINGLE_FILE
	!ifdef MAKE_UNINSTALLER
		OutFile "Release\MakeUnInstaller.exe"
	!else
		; One big install file, do not remove upon completion
		OutFile "Release\COHSetup.exe"
	!endif
!else
	; Just a small setup file, which gets wrapped, and removed upon completion
	!define SETUP_NAME "CohSetup.exe"
	; The file to write
	OutFile "${SETUP_NAME}"
!endif

; The default installation directory
; This gets overridden in onGUIinit with a localized version
!ifdef COV_INSTALLER
	InstallDir "$PROGRAMFILES\City of Heroes"
!else
	!ifdef KOREAN_ONLY
		InstallDir "$PROGRAMFILES\NCSOFT\$(^NAME)"
	!else
		!ifdef COMBO_INSTALLER
			InstallDir "$PROGRAMFILES\$(CoHName)"
		!else
			InstallDir "$PROGRAMFILES\$(^NAME)"
		!endif
	!endif
!endif
	
; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKCU SOFTWARE\Cryptic\${GAME_REGISTRY_KEY} "Installation Directory"

; The text to prompt the user to enter a directory
ComponentText "$(DESC_ComponentText)"

; The text to prompt the user to enter a directory
DirText $(DESC_DirText1) $(DESC_DirText2) $(DESC_DirText3) $(DESC_DirText4)

;ShowInstDetails show

;--------------------------------
;Modern UI Configuration

!define MUI_HEADERIMAGE
;  !define MUI_ABORTWARNING

;--------------------------------
;Variables

Var MUI_TEMP
Var STARTMENU_FOLDER
Var STARTED_MUSIC
Var EXTRACTED_MUSIC
!ifndef SINGLE_FILE
	Var CD_ROOT
!endif
Var PREVIOUS_INSTALL
Var UNINST_REG_KEY
	
;--------------------------------
;Pages

	!insertmacro MUI_PAGE_LICENSE $(transLicenseData)
	!insertmacro MUI_PAGE_COMPONENTS
!define MUI_PAGE_CUSTOMFUNCTION_SHOW .onDirectoryPageShow
	!insertmacro MUI_PAGE_DIRECTORY

	;Start Menu Folder Page Configuration
	!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
	!ifdef COV_INSTALLER
		!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Cryptic\${GAME_REGISTRY_KEY}\CoV" 
	!else
		!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Cryptic\${GAME_REGISTRY_KEY}" 
	!endif
	!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
	  
	!insertmacro MUI_PAGE_STARTMENU ${GAME_REGISTRY_KEY} $STARTMENU_FOLDER
	!insertmacro MUI_PAGE_INSTFILES
!define MUI_PAGE_CUSTOMFUNCTION_SHOW .onFinishPageShow
	!insertmacro MUI_PAGE_FINISH

	; Uninstall stuff  
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES


 
;--------------------------------
;Installer Defines

!ifndef SINGLE_FILE
	!define DISC1_FILENAME "${DATA_FILE_BASENAME}1.exe"
	!define DISC1_INSTALLER "$CD_ROOT\${DISC1_FILENAME}"
	!define DISC2_FILENAME "${DATA_FILE_BASENAME}2.exe"
	!define DISC2_INSTALLER "$CD_ROOT\${DISC2_FILENAME}"
	!define DISC3_FILENAME "${DATA_FILE_BASENAME}3.exe"
	!define DISC3_INSTALLER "$CD_ROOT\${DISC3_FILENAME}"
	!define DISC4_FILENAME "${DATA_FILE_BASENAME}4.exe"
	!define DISC4_INSTALLER "$CD_ROOT\${DISC4_FILENAME}"
	!define DISC5_FILENAME "${DATA_FILE_BASENAME}5.exe"
	!define DISC5_INSTALLER "$CD_ROOT\${DISC5_FILENAME}"
!endif

;--------------------------------
;Languages Include

!include "Languages.nsh"

!insertmacro INHERIT_LANGUAGE_FROM_WRAPPER

 
Function GetDXVersionString
	Push $0

	ReadRegStr $0 HKLM "Software\Microsoft\DirectX" "Version"
	IfErrors noDirectX
	
	StrCpy $0 $0 5 2

	Goto done

	noDirectX:
		StrCpy $0 "0.00"

	done:
		Exch $0
FunctionEnd

Function GetDXVersionStringNice
	Push $0
	Push $1

	ReadRegStr $0 HKLM "Software\Microsoft\DirectX" "Version"
	IfErrors noDirectX
	
	StrCpy $0 $0 5 2
	StrCpy $1 $0 1 0
	StrCmp $1 "0" FirstCharIsZero done
	
	FirstCharIsZero:
		StrCpy $0 $0 4 1	
		Goto done

	noDirectX:
		StrCpy $0 "0.00"

	done:
		Pop $1
		Exch $0
FunctionEnd

Function GetDXVersion
	Push $0
	Push $1

	Call GetDXVersionString
	Pop $0

	StrCpy $1 $0 2 3    ; get the minor version
	StrCpy $0 $0 2 0    ; get the major version
	IntOp $0 $0 * 100   ; $0 = major * 100 + minor
	IntOp $0 $0 + $1

	Pop $1
	Exch $0
FunctionEnd

Var DIRECTX_VERSION

Function CheckDXVersion
	Call GetDXVersion
	Pop $R3
	; Check against DirectX 8, that is really all we require
	IntCmp $R3 800 okay 0 okay
		Call GetDXVersionStringNice
		Pop $DIRECTX_VERSION
		!ifdef SINGLE_FILE
			MessageBox "MB_YESNO" $(PROMPT_OldDirectXVersion_Download) IDNO DoCancel IDYES SkipDirectXInstall
		!else
			MessageBox "MB_YESNOCANCEL" $(PROMPT_OldDirectXVersion) IDNO SkipDirectXInstall IDCANCEL DoCancel
			;InstallDirectX:   ; fall-through case
				MessageBox "MB_OK" $(ALERT_AfterDirectXRerun)
				Exec "$CD_ROOT\DirectX90c\dxsetup.exe"
				Abort
		!endif
		DoCancel:
			MessageBox "MB_OK" $(ERROR_CancelOnDirectX)
			Abort

	SkipDirectXInstall:
	okay:

FunctionEnd


Function onGUIInit_Custom

	; see if there was a previous install of city of heroes
	!ifdef GR_BETA_INSTALLER
		!ifdef ENGLISH_ONLY
			ReadRegStr $R1 HKCU SOFTWARE\Cryptic\CohBeta "Installation Directory"
		!else
			ReadRegStr $R1 HKCU SOFTWARE\Cryptic\CohTest "Installation Directory"
		!endif
	!else ifdef BETA_INSTALLER
		ReadRegStr $R1 HKCU SOFTWARE\Cryptic\CohTest "Installation Directory"
	!else
		ReadRegStr $R1 HKCU SOFTWARE\Cryptic\${GAME_REGISTRY_KEY} "Installation Directory"
	!endif
	StrCmp $R1 "" noPreviousInstall previousInstall
	
	; found a previous install, use this install directory as the default
	previousInstall:

		IntOp $PREVIOUS_INSTALL 1 + 0

		StrCpy $INSTDIR $R1
		Goto endFindInstallDir

	; did not find a previous install, use city of heroes as the default
	noPreviousInstall:

		IntOp $PREVIOUS_INSTALL 0 + 0

		IfErrors 0 0 ;clearing the error flag
		!ifdef GR_BETA_INSTALLER
			!ifdef ENGLISH_ONLY
				StrCpy $INSTDIR "$PROGRAMFILES\CohBeta"
			!else
				StrCpy $INSTDIR "$PROGRAMFILES\CohTest"
			!endif
		!else ifdef BETA_INSTALLER
			StrCpy $INSTDIR "$PROGRAMFILES\CohTest"
		!else
			!ifdef KOREAN_ONLY
				StrCpy $INSTDIR "$PROGRAMFILES\NCSOFT\$(^NAME)"
			!else
				StrCpy $INSTDIR "$PROGRAMFILES\City of Heroes"
			!endif
		!endif
	

	endFindInstallDir:
	
	# Set the appropriate default start menu entry
	!ifdef KOREAN_ONLY
		StrCpy $STARTMENU_FOLDER "NCSOFT\$(^NAME)"
	!else
		!ifdef COMBO_INSTALLER
			StrCpy $STARTMENU_FOLDER "$(CoHName)"
		!else
			StrCpy $STARTMENU_FOLDER "$(^Name)"
		!endif
	!endif
	!ifdef GR_BETA_INSTALLER
		StrCpy $STARTMENU_FOLDER "$STARTMENU_FOLDER Beta"
	!else ifdef BETA_INSTALLER
		StrCpy $STARTMENU_FOLDER "$STARTMENU_FOLDER Beta"
	!endif

 
    !ifdef SINGLE_FILE
		# No CD_ROOT variable
    !else
		# Set the CD_ROOT variable from the command line
		
		Call GetParameters
		Pop $CD_ROOT

		StrCmp $CD_ROOT "" BadParam

		IfFileExists "$CD_ROOT\FinishSetup.exe" GoodParam

		BadParam:
			MessageBox MB_ICONSTOP "$(ERROR_WrongExecutable)"
			Quit
		GoodParam:
	!endif

	# the plugins dir is automatically deleted when the installer exits
	InitPluginsDir
	
	; Make sure only one copy of the installer is running at a time.
	!ifndef DEBUG
		System::Call 'kernel32::CreateMutexA(i 0, i 0, t "CityOfHeroesInstallerMutex") i .r1 ?e' 
		Pop $R0 
		StrCmp $R0 0 +3 
			MessageBox MB_OK $(ERROR_InstallerAlreadyRunning)
			Abort
	!endif
		
	IfErrors GotError
	
	# Search for the current DirectX Version
	Call CheckDXVersion	

	; BGImage stuff
	!ifndef DISABLE_BGIMAGE

	!ifdef DEBUG
		# turn return values on if in debug mode
		BgImage::SetReturn /NOUNLOAD on
	!endif
		IfErrors GotError

	!ifdef GR_INSTALLER
		!insertmacro SPLASHIMAGE GR_Splash_Screen
	!else ifdef COV_INSTALLER
		!insertmacro SPLASHIMAGE VSplash_Screen
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE Splash_Screen_Korean
		!else
			!insertmacro SPLASHIMAGE Splash_Screen
		!endif
	!endif
		
	#	# set the initial background for images to be drawn on
	#	# we will use a gradient
	#	!insertmacro SPLASHIMAGEGRADIENT
	#	IfErrors GotError
		
	#	# show our creation to the world!
	#	BgImage::Redraw /NOUNLOAD
	#	# Refresh doesn't return any value
	#	IfErrors GotError

		Goto End
		
		GotError:
	;	MessageBox MB_OK "Error in init"
		
		
		End:

	!endif ; DISABLE_BGIMAGE

	IntOp $STARTED_MUSIC 0 + 0
	IntOp $EXTRACTED_MUSIC 0 + 0
  
FunctionEnd ; .onGUIInit


Function .onFinishPageShow
	!ifndef DISABLE_BGIMAGE
		BgImage::Sound /NOUNLOAD /STOP
	!endif
	
!ifndef KOREAN_ONLY
	!ifndef GR_BETA_INSTALLER
		!ifndef BETA_INSTALLER
			MessageBox MB_YESNO "$(PROMPT_PlayNC)" IDNO doNothing
			
			ExecShell "open" "$(RegisterLink)"
			
			doNothing:
		!endif
	!endif
!endif

FunctionEnd


Function .onDirectoryPageShow
	IntCmp $PREVIOUS_INSTALL 1 HasPreviousInstall
	Goto End
	
	HasPreviousInstall:
		!ifndef KOREAN_ONLY
			; Korea doesn't get this because they will not have had CoH installed previously
			MessageBox MB_OK "$(PROMPT_PreviousInstallDetected)"
		!endif
	
	End:

FunctionEnd



Function .onGUIEnd

	!ifndef DISABLE_BGIMAGE
		BgImage::Sound /NOUNLOAD /STOP
		# Destroy must not have /NOUNLOAD so NSIS will be able to unload
		# and delete BgImage before it exits
		BgImage::Destroy
		# Destroy doesn't return any value
	!endif ; DISABLE_BGIMAGE
	
!ifndef SINGLE_FILE ; SINGLE_FILE has no temporary setup file
	; Delete tempoary setup file
	IfFileExists "$CD_ROOT\FinishSetup.exe" FinishSetup RebootDelete
	FinishSetup:
		Exec "$CD_ROOT\FinishSetup.exe $EXEDIR"
		IfErrors RebootDelete
		goto Done
	RebootDelete:
		Delete /REBOOTOK "$EXEDIR\${SETUP_NAME}"
!endif

	Done:
	
FunctionEnd


Function loadBgMusic
	IntCmp $EXTRACTED_MUSIC 1 Done
	IntOp $EXTRACTED_MUSIC 1 + 0
	!ifndef DISABLE_BGIMAGE
		; Extract BGLoop
		!ifdef GR_INSTALLER
			File /oname=$PLUGINSDIR\P_Install_loop.wav "C:\Game\Src\Installer\GR_Resources\P_Install_Loop.wav"
		!else ifdef COV_INSTALLER
			File /oname=$PLUGINSDIR\V_Install_loop.wav "C:\Game\Src\Installer\V_Resources\V_Install_loop.wav"
		!else
			File /oname=$PLUGINSDIR\Install_loop.wav "C:\Game\Src\Installer\Resources\Install_loop.wav"
		!endif
	!endif
	Done:
FunctionEnd


Function playBgMusic
    Call loadBgMusic

	IntCmp $STARTED_MUSIC 1 Done
	IntOp $STARTED_MUSIC 1 + 0
	!ifndef DISABLE_BGIMAGE
		!ifdef GR_INSTALLER
			BgImage::Sound /NOUNLOAD /LOOP $PLUGINSDIR\P_Install_loop.wav
		!else ifdef COV_INSTALLER
			BgImage::Sound /NOUNLOAD /LOOP $PLUGINSDIR\V_Install_loop.wav
		!else
			BgImage::Sound /NOUNLOAD /LOOP $PLUGINSDIR\Install_loop.wav
		!endif
		
		!insertmacro GetReturnValue
	!endif
	Done:
FunctionEnd




Function hideWindow
	System::Call "User32::ShowWindow(i, i) b ($HWNDPARENT, ${SW_HIDE})"
FunctionEnd

Function showWindow
	System::Call "User32::ShowWindow(i, i) b ($HWNDPARENT, ${SW_RESTORE})"
	System::Call "User32::SetForegroundWindow(i) b ($HWNDPARENT)"
FunctionEnd




;--------------------------------

; The stuff to install
# SecMainName is defined in game\src\lang\english.nsh
# e.g. LangString SecMainName ${LANG_ENGLISH} "$(^NAME) (required)"
Section $(SecMainName) SecMain

	SectionIn RO
	ClearErrors
	  
	!ifndef SINGLE_FILE
		; Determine size of the Data Section
		!system "Release\CD1\${DATA_FILE_BASENAME}1.exe /SECRET"
		!include "DataSize1.nsh"
		!system "Release\CD2\${DATA_FILE_BASENAME}2.exe /SECRET"
		!include "DataSize2.nsh"
		!system "Release\CD3\${DATA_FILE_BASENAME}3.exe /SECRET"
		!include "DataSize3.nsh"
		!system "Release\CD4\${DATA_FILE_BASENAME}4.exe /SECRET"
		!include "DataSize4.nsh"
		!system "Release\CD5\${DATA_FILE_BASENAME}5.exe /SECRET"
		!include "DataSize5.nsh"
	!endif

	; Call repositionWindow
	  
	; Set output path to the installation directory.
	SetOutPath $INSTDIR
	  
	Call playBgMusic
	ClearErrors

	; Copy the updater first, and based on whether its English or European
	File /oname=${UPDATER_OUTPUT_NAME} "${UPDATER_SOURCE_DIR}\${UPDATER_EXE_NAME}"

	; Install Uninstaller *before* copying lots of data

	;!ifndef DEBUG
		; Write the installation path into the registry
		WriteRegStr HKCU SOFTWARE\Cryptic\${GAME_REGISTRY_KEY} "Installation Directory" "$INSTDIR"
		WriteRegStr HKCU SOFTWARE\Cryptic\${GAME_REGISTRY_KEY} "CurrentVersion" "${VERSION}"
	;!endif

	; Write the uninstall keys for Windows
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_REGISTRY_KEY}" "DisplayName" "$(UninstallRegName)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_REGISTRY_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
	  
	!ifdef MAKE_UNINSTALLER
		SetFileAttributes "uninstall.exe" NORMAL
		ClearErrors ;if the updater doesnt exist yet, the previous line will generate an error, which will cause disk 1 to fail when it checks for errors later
		WriteUninstaller "uninstall.exe"
	!else
		File "${SOURCE_DIR}\uninstall.exe"
	!endif

    !ifdef MAKE_UPDATER_ONLY
           ; no files here. updater already copied.
	   ; Ask for enough space to download all the files
	   AddSize 4000000
	!else ifdef SINGLE_FILE
		call repositionWindowForSingleFile
		; Single file version
		!include "Coh.files1.nsh"
		!include "Coh.files2.nsh"
		!include "Coh.files3.nsh"
		!include "Coh.files4.nsh"
		;!include "Coh.files5.nsh"
		call centerWindow
		
	!else
		; Multiple files on multiple disks

		Call hideWindow

		; Prompt for insert 1st disc

		LookForDisc1:
			; Look for second image in same directory (network install or disc has switched)
			StrCpy $0 "${DISC1_INSTALLER}"
			IfFileExists "$0" DiscReadyDoit1
			; Look for second image in relative directory
			StrCpy $0 "$CD_ROOT\..\CD1\${DISC1_FILENAME}"
			IfFileExists "$0" DiscReadyDoit1
			StrCpy $0 "$CD_ROOT\..\Disc1\${DISC1_FILENAME}"
			IfFileExists "$0" DiscReadyDoit1
			; Look for second image in other CDRom drives
			Push "${DISC1_FILENAME}"
			Call FindFileInRoot
			Pop $0
			StrCmp $0 "NOT FOUND" InsertCD1 DiscReadyDoit1
		  
		InsertCD1:
			DetailPrint "$(PROMPT_InsertDisc1)"
			MessageBox "MB_OKCANCEL" "$(PROMPT_InsertDisc1)" IDCANCEL InstallCancelled
			Goto LookForDisc1
			
		DiscReadyDoit1:
		ExecWait '"$0" $INSTDIR'

		Call showWindow
		IfErrors ErrorCopying1  NoError1

		ErrorCopying1:
			DetailPrint $(DEBUG_ErrorExtracting1)
			Abort

		NoError1:

		!ifdef CD_COUNT_1
			Goto DoneCopyingFromDisks
		!endif

		; Prompt for insert 2nd disc
		  
		LookForDisc2:
			; Look for second image in same directory (network install or disc has switched)
			StrCpy $0 "${DISC2_INSTALLER}"
			IfFileExists "$0" DiscReadyDoit2
			; Look for second image in relative directory
			StrCpy $0 "$CD_ROOT\..\CD2\${DISC2_FILENAME}"
			IfFileExists "$0" DiscReadyDoit2
			StrCpy $0 "$CD_ROOT\..\Disc2\${DISC2_FILENAME}"
			IfFileExists "$0" DiscReadyDoit2
			; Look for second image in other CDRom drives
			Push "${DISC2_FILENAME}"
			Call FindFileInRoot
			Pop $0
			StrCmp $0 "NOT FOUND" InsertCD2 DiscReadyDoit2
		  
		InsertCD2:
			DetailPrint "$(PROMPT_InsertDisc2)"
			MessageBox "MB_OKCANCEL" "$(PROMPT_InsertDisc2)" IDCANCEL InstallCancelled
			Goto LookForDisc2
			
		DiscReadyDoit2:
		ExecWait '"$0" $INSTDIR'

		Call showWindow
		IfErrors ErrorCopying2  NoError2
		  
		ErrorCopying2:
			DetailPrint $(DEBUG_ErrorExtracting2)
			Abort
		  
		NoError2:

		!ifdef CD_COUNT_2
			Goto DoneCopyingFromDisks
		!endif

		; Prompt for insert 3rd disc
		  
		LookForDisc3:
			; Look for 3rd image in same directory (network install or disc has switched)
			StrCpy $0 "${DISC3_INSTALLER}"
			IfFileExists "$0" DiscReadyDoit3
			; Look for second image in relative directory
			StrCpy $0 "$CD_ROOT\..\CD3\${DISC3_FILENAME}"
			IfFileExists "$0" DiscReadyDoit3
			StrCpy $0 "$CD_ROOT\..\Disc3\${DISC3_FILENAME}"
			IfFileExists "$0" DiscReadyDoit3
			; Look for second image in other CDRom drives
			Push "${DISC3_FILENAME}"
			Call FindFileInRoot
			Pop $0
			StrCmp $0 "NOT FOUND" InsertCD3 DiscReadyDoit3
		  
		InsertCD3:
			DetailPrint "$(PROMPT_InsertDisc3)"
			MessageBox "MB_OKCANCEL" "$(PROMPT_InsertDisc3)" IDCANCEL InstallCancelled
			Goto LookForDisc3
		  
		DiscReadyDoit3:
		ExecWait '"$0" $INSTDIR'

		Call showWindow
		IfErrors ErrorCopying3  NoError3
		  
		ErrorCopying3:
			DetailPrint $(DEBUG_ErrorExtracting3)
			Abort
		  
		NoError3:

		!ifdef CD_COUNT_3
			Goto DoneCopyingFromDisks
		!endif

		; Prompt for insert 4th disc
		  
		LookForDisc4:
			; Look for 4th image in same directory (network install or disc has switched)
			StrCpy $0 "${DISC4_INSTALLER}"
			IfFileExists "$0" DiscReadyDoit4
			; Look for second image in relative directory
			StrCpy $0 "$CD_ROOT\..\CD4\${DISC4_FILENAME}"
			IfFileExists "$0" DiscReadyDoit4
			StrCpy $0 "$CD_ROOT\..\Disc4\${DISC4_FILENAME}"
			IfFileExists "$0" DiscReadyDoit4
			; Look for second image in other CDRom drives
			Push "${DISC4_FILENAME}"
			Call FindFileInRoot
			Pop $0
			StrCmp $0 "NOT FOUND" InsertCD4 DiscReadyDoit4
		  
		InsertCD4:
			DetailPrint "$(PROMPT_InsertDisc4)"
			MessageBox "MB_OKCANCEL" "$(PROMPT_InsertDisc4)" IDCANCEL InstallCancelled
			Goto LookForDisc4
			
		DiscReadyDoit4:
		ExecWait '"$0" $INSTDIR'

		Call showWindow
		IfErrors ErrorCopying4  NoError4
		  
		ErrorCopying4:
			DetailPrint $(DEBUG_ErrorExtracting4)
			Abort

		NoError4:

		!ifdef CD_COUNT_4
			Goto DoneCopyingFromDisks
		!endif

		; Prompt for insert 5th disc
		  
		LookForDisc5:
			; Look for 5th image in same directory (network install or disc has switched)
			StrCpy $0 "${DISC5_INSTALLER}"
			IfFileExists "$0" DiscReadyDoit5
			; Look for second image in relative directory
			StrCpy $0 "$CD_ROOT\..\CD5\${DISC5_FILENAME}"
			IfFileExists "$0" DiscReadyDoit5
			StrCpy $0 "$CD_ROOT\..\Disc5\${DISC5_FILENAME}"
			IfFileExists "$0" DiscReadyDoit5
			; Look for second image in other CDRom drives
			Push "${DISC5_FILENAME}"
			Call FindFileInRoot
			Pop $0
			StrCmp $0 "NOT FOUND" InsertCD5 DiscReadyDoit5
		  
		InsertCD5:
			DetailPrint "$(PROMPT_InsertDisc5)"
			MessageBox "MB_OKCANCEL" "$(PROMPT_InsertDisc5)" IDCANCEL InstallCancelled
			Goto LookForDisc5
			
		DiscReadyDoit5:
		ExecWait '"$0" $INSTDIR'

		Call showWindow
		IfErrors ErrorCopying5  NoError5
		  
		ErrorCopying5:
			DetailPrint $(DEBUG_ErrorExtracting5)
			Abort

		; the installation was canceled
		InstallCancelled:
			MessageBox "MB_OK" "$(ERROR_Disc2Cancelled)"
			DetailPrint $(DEBUG_ErrorFindingDisc2)
			Abort
		  
		NoError5:

		DoneCopyingFromDisks:
	  
	!endif ; not SINGLE_FILE


	SetOutPath $INSTDIR ; Set working directory for shortcuts

	!insertmacro MUI_STARTMENU_WRITE_BEGIN ${GAME_REGISTRY_KEY}

		;Create shortcuts
		CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
	!ifdef COV_INSTALLER
		CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(UninstallName).lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\CityOfHeroes.exe" 2
		; use the CoVBetaUpdater for beta installers, else use the CoVUpdater
		!ifdef BETA_INSTALLER
			CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(^NAME).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "" "$INSTDIR\CityOfHeroes.exe" 2
		!else
			CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(^NAME).lnk" "$INSTDIR\${COV_UPDATER_OUTPUT_NAME}" "" "$INSTDIR\CityOfHeroes.exe" 2
			CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(RegisterLinkName).lnk" "$(RegisterLink)"
		!endif
		CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(WebsiteLinkName).lnk" "$(WebsiteLink)"
	!else
		!ifdef KOREAN_ONLY
			CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(UninstallName).lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\CityOfHeroes.exe" 3
			CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(^NAME).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "" "$INSTDIR\CityOfHeroes.exe" 3
			CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(WebsiteLinkName).lnk" "$(WebsiteLink)"
		!else
			CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(UninstallName).lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
			!ifdef GR_INSTALLER
				!ifdef GR_BETA_INSTALLER
					!ifdef ENGLISH_ONLY
						CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(CoHName).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "-uiskin 2 -beta" "$INSTDIR\${UPDATER_OUTPUT_NAME}" 0
					!else
						CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(CoHName).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "-uiskin 2 -test" "$INSTDIR\${UPDATER_OUTPUT_NAME}" 0
					!endif
				!else
					!ifdef ENGLISH_ONLY
						CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(CoHName).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "-uiskin 2" "$INSTDIR\${UPDATER_OUTPUT_NAME}" 0
					!endif
				!endif
			!else ifdef COMBO_INSTALLER
				CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(CoHName).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "" "$INSTDIR\CityOfHeroes.exe" 0
			!else
				CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(^NAME).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "" "$INSTDIR\CityOfHeroes.exe" 0
			!endif
			CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(WebsiteLinkName).lnk" "$(WebsiteLink)"
			CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(RegisterLinkName).lnk" "$(RegisterLink)"
		!endif
	!endif
	  
	!insertmacro MUI_STARTMENU_WRITE_END

	!ifndef DISABLE_BGIMAGE
		# Destroy must not have /NOUNLOAD so NSIS will be able to unload
		# and delete BgImage before it exits
		BgImage::Destroy
		# Destroy doesn't return any value
	!endif ; DISABLE_BGIMAGE
	
SectionEnd



Section $(SecDShortcutsName) SecDShortcuts

	SectionIn 2

	SetOutPath $INSTDIR ; Set working directory for shortcuts

	!ifdef GR_INSTALLER
		!ifdef GR_BETA_INSTALLER
			!ifdef ENGLISH_ONLY
				CreateShortCut "$DESKTOP\$(CoHName).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "-uiskin 2 -beta" "$INSTDIR\${UPDATER_OUTPUT_NAME}" 0
			!else
				CreateShortCut "$DESKTOP\$(CoHName).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "-uiskin 2 -test" "$INSTDIR\${UPDATER_OUTPUT_NAME}" 0
			!endif
		!else
			CreateShortCut "$DESKTOP\$(CoHName).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "-uiskin 2" "$INSTDIR\${UPDATER_OUTPUT_NAME}" 0
		!endif
	!else ifdef COV_INSTALLER
		!ifdef BETA_INSTALLER
			CreateShortCut "$DESKTOP\$(CoVName) Beta.lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "" "$INSTDIR\CityOfHeroes.exe" 2
		!else
			CreateShortCut "$DESKTOP\$(CoVName).lnk" "$INSTDIR\${COV_UPDATER_OUTPUT_NAME}" "" "$INSTDIR\CityOfHeroes.exe" 2
		!endif
	!else
		!ifdef KOREAN_ONLY
			CreateShortCut "$DESKTOP\$(^NAME).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "" "$INSTDIR\CityOfHeroes.exe" 3
		!else
			CreateShortCut "$DESKTOP\$(CoHName).lnk" "$INSTDIR\${UPDATER_OUTPUT_NAME}" "" "$INSTDIR\CityOfHeroes.exe" 0
		!endif
	!endif

    !ifdef BESTBUY_INSTALLER    
       CreateShortcut "$DESKTOP\Best Buy Games.lnk" "http://www.bestbuygames.com/" "" "$OUTDIR\piggs\Best_Buy.ico" 0
    !endif
  
SectionEnd



;--------------------------------

; Uninstaller

; Uninstall section

Section "Uninstall"


	; find out which installation (coh, eucoh, training room) we are uninstalling 
	; to remove the correct registry keys, start menu entries
	ReadRegStr $R1 HKCU SOFTWARE\Cryptic\COH "Installation Directory"
	StrCmp $R1 $INSTDIR detectedCoh
	ReadRegStr $R1 HKCU SOFTWARE\Cryptic\COHTest "Installation Directory"
	StrCmp $R1 $INSTDIR detectedCohTest
	ReadRegStr $R1 HKCU SOFTWARE\Cryptic\EUCOH "Installation Directory"
	StrCmp $R1 $INSTDIR detectedEUCoh
	ReadRegStr $R1 HKCU SOFTWARE\Cryptic\EUCOHTest "Installation Directory"
	StrCmp $R1 $INSTDIR detectedEUCohTest
	ReadRegStr $R1 HKCU SOFTWARE\Cryptic\KRCOH "Installation Directory"
	StrCmp $R1 $INSTDIR detectedKRCoh

	detectedCoh:
		StrCpy $UNINST_REG_KEY "COH"
	Goto doneDetectedRegistryKeys

	detectedEUCoh:
		StrCpy $UNINST_REG_KEY "EUCOH"
	Goto doneDetectedRegistryKeys

	detectedKRCoh:
		StrCpy $UNINST_REG_KEY "KRCOH"
	Goto doneDetectedRegistryKeys

	detectedCohTest:
		StrCpy $UNINST_REG_KEY "COHTest"
	Goto doneDetectedRegistryKeys

	detectedEUCohTest:
		StrCpy $UNINST_REG_KEY "EUCOHTest"
	Goto doneDetectedRegistryKeys

	doneDetectedRegistryKeys:

	ClearErrors

	DoRetry:  
		; remove files
		Delete $INSTDIR\CityOfHeroes.exe
		Delete $INSTDIR\CityOfHeroes.pdb
		Delete $INSTDIR\PatchClient.exe
		Delete $INSTDIR\PatchClient.exe.bak
		Delete $INSTDIR\CohUpdater.exe
		Delete $INSTDIR\CohUpdater.pdb
		Delete $INSTDIR\CohUpdater.exe.bak
		Delete $INSTDIR\CohUpdater.old
		Delete $INSTDIR\CohUpdater.new
		Delete $INSTDIR\CohUpdater.tmp
		Delete $INSTDIR\CohUpdater.eu.exe
		Delete $INSTDIR\CohUpdater.eu.pdb
		Delete $INSTDIR\CohUpdater.kr.exe
		Delete $INSTDIR\CohUpdater.kr.exe.bak
		Delete $INSTDIR\CohUpdater.kr.pdb
		Delete $INSTDIR\CohUpdater.eu.exe.bak
		Delete $INSTDIR\CohUpdater.eu.old
		Delete $INSTDIR\CohUpdater.eu.new
		Delete $INSTDIR\CohUpdater.eu.tmp
		Delete $INSTDIR\CovUpdater.exe
		Delete $INSTDIR\CovUpdater.eu.exe
		Delete $INSTDIR\CovUpdater.kr.exe
		Delete $INSTDIR\CoVBetaUpdater.exe
		Delete $INSTDIR\${UPDATER_OUTPUT_NAME}
		Delete $INSTDIR\${UPDATER_OUTPUT_NAME}.bak
		Delete $INSTDIR\updater.lock
		Delete $INSTDIR\cmdline.txt
		Delete $INSTDIR\config.txt
		Delete $INSTDIR\*.mdmp
		Delete $INSTDIR\*.checksum
		Delete $INSTDIR\*.patch
		Delete $INSTDIR\*.majorpatch
		Delete $INSTDIR\*.manifest
		Delete $INSTDIR\dfengine.dll
		Delete $INSTDIR\dbghelp.dll
		Delete $INSTDIR\CrashRpt.dll
		Delete $INSTDIR\CrashRpt.pdb
		Delete $INSTDIR\tempfile
		Delete $INSTDIR\README*.*
		Delete $INSTDIR\dxdiag.html
		Delete $INSTDIR\CrashDescription.txt
		Delete $INSTDIR\cg.dll
		Delete $INSTDIR\cggl.dll
		Delete $INSTDIR\NxCooking.dll
		Delete $INSTDIR\NxPhysics.dll
		Delete $INSTDIR\NxCore.dll
		Delete $INSTDIR\PhysXCore.dll
		Delete $INSTDIR\PhysXLoader.dll
		Delete $INSTDIR\cohupdater.prv
		Delete $INSTDIR\CohUpdater_UI_Win.dll

		Delete $INSTDIR\cider\*.html

		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_128_512\df.dat
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_128_512\generic_tech_wall_128_512.tga
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_128\df.dat
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_128\generic_tech_wall_256_128.tga
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_256\df.dat
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_256\generic_tech_wall_256_256.tga
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_512\df.dat
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_512\generic_tech_wall_256_512.tga
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_1024\df.dat
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_1024\generic_tech_wall_256_1024.tga
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_512_128\df.dat
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_512_128\generic_tech_wall_512_128.tga
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_512_256\df.dat
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_512_256\generic_tech_wall_512_256.tga
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_512_512\df.dat
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_512_512\generic_tech_wall_512_512.tga
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_1024_256\df.dat
		Delete $INSTDIR\doublefusion\cache\data\generic_tech_wall_1024_256\generic_tech_wall_1024_256.tga
		Delete $INSTDIR\doublefusion\cache\objects.dfo
		Delete $INSTDIR\doublefusion\engine.var

		Delete $INSTDIR\logs\game\*.log
		Delete $INSTDIR\logs\*.log
		Delete $INSTDIR\client_demos\*.cohdemo

		Delete $INSTDIR\piggs\*.pigg
		Delete $INSTDIR\piggs\*.checksum
		  
		Delete $INSTDIR\Uninstall.exe ; Windows copies the uninstaller to a temp directory, so we can freely delete this while running
		  
		; If any errors occurred while deleting, then raise a ruckus!  Allow them to retry or cancel
		IfErrors ErrorDeleting  NoErrors
  
	ErrorDeleting:
		MessageBox MB_RETRYCANCEL $(ERROR_CouldNotDeleteAllFiles) IDRETRY DoRetry IDCANCEL DoCancel

	DoCancel:
		Abort
  
	NoErrors:
  
	; remove the data\geobin folder and any files that may be in it
	RMDir /r "$INSTDIR\piggs\geobin"
	; remove the directories (if empty)
	RMDir "$INSTDIR\piggs"
	RMDir "$INSTDIR\logs\game"
	RMDir "$INSTDIR\logs"
	RMDir "$INSTDIR\doublefusion\cache\data\generic_tech_wall_128_512"
	RMDir "$INSTDIR\doublefusion\cache\data\generic_tech_wall_256_128"
	RMDir "$INSTDIR\doublefusion\cache\data\generic_tech_wall_256_256"
	RMDir "$INSTDIR\doublefusion\cache\data\generic_tech_wall_256_512"
	RMDir "$INSTDIR\doublefusion\cache\data\generic_tech_wall_256_1024"
	RMDir "$INSTDIR\doublefusion\cache\data\generic_tech_wall_512_128"
	RMDir "$INSTDIR\doublefusion\cache\data\generic_tech_wall_512_256"
	RMDir "$INSTDIR\doublefusion\cache\data\generic_tech_wall_512_512"
	RMDir "$INSTDIR\doublefusion\cache\data\generic_tech_wall_1024_256"
	RMDir "$INSTDIR\doublefusion\cache\data"
	RMDir "$INSTDIR\doublefusion\cache"
	RMDir "$INSTDIR\doublefusion"
	RMDir "$INSTDIR\cider"
	RMDir "$INSTDIR\client_demos"
	RMDir "$INSTDIR"

	; skip deleting shortcuts and start menu items if this is CohTest
	ReadRegStr $R1 HKCU SOFTWARE\Cryptic\CohTest "Installation Directory"
	StrCmp $R1 $INSTDIR deletingCohTest
	ReadRegStr $R1 HKCU SOFTWARE\Cryptic\EUCohTest "Installation Directory"
	StrCmp $R1 $INSTDIR deletingEUCohTest

	; errors were set when trying to ReadRegStr
	ClearErrors

	; Delete Desktop shortcut
	Delete "$DESKTOP\$(^NAME).lnk"
	Delete "$DESKTOP\$(CoHName).lnk"
	Delete "$DESKTOP\$(CoVName).lnk"
	Delete "$DESKTOP\${KOREAN_NAME}.lnk"

	; If any errors occurred while deleting, then raise a ruckus!  Allow them to retry or cancel
	IfErrors ErrorDeleting

	; Delete CoV Start Menu entries
	ReadRegStr $MUI_TEMP HKCU "Software\Cryptic\$UNINST_REG_KEY\CoV" "Start Menu Folder"

	Delete "$SMPROGRAMS\$MUI_TEMP\$(UninstallName).lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\$(^NAME).lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\$(WebsiteLinkName).lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\$(RegisterLinkName).lnk"

	; Remove CoH Start Menu entries
	ReadRegStr $MUI_TEMP HKCU "Software\Cryptic\$UNINST_REG_KEY" "Start Menu Folder"

	Delete "$SMPROGRAMS\$MUI_TEMP\$(UninstallName).lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\$(CoHName).lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\$(WebsiteLinkNameCoH).lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\$(RegisterLinkName).lnk"
	
	; Remove Korean entries regardless of locale, if found
	Delete "$SMPROGRAMS\$MUI_TEMP\${KOREAN_NAME}.lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\${KOREAN_UNINSTALL_NAME}.lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\${KOREAN_WEBSITE_NAME}.lnk"

	;Delete empty start menu parent diretories
	StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"

	ClearErrors

	startMenuDeleteLoop:
		RMDir $MUI_TEMP
		GetFullPathName $MUI_TEMP "$MUI_TEMP\.."

		IfErrors startMenuDeleteLoopDone

		StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
	startMenuDeleteLoopDone:

	; remove directories used (if empty)
	RMDir "$SMPROGRAMS\$(^NAME)"
	RMDir "$INSTDIR\piggs"
	RMDir "$INSTDIR\logs\game"
	RMDir "$INSTDIR\logs"
	RMDir "$INSTDIR\client_demos"
	RMDir "$INSTDIR"


	; find out which installation (coh, eucoh, training room) we are uninstalling 
	; to remove the correct registry keys
	ReadRegStr $R1 HKCU SOFTWARE\Cryptic\COH "Installation Directory"
	StrCmp $R1 $INSTDIR deletingCoh 
	ReadRegStr $R1 HKCU SOFTWARE\Cryptic\EUCOH "Installation Directory"
	StrCmp $R1 $INSTDIR deletingEUCoh
	ReadRegStr $R1 HKCU SOFTWARE\Cryptic\KRCOH "Installation Directory"
	StrCmp $R1 $INSTDIR deletingKRCoh

	deletingCoh:
		StrCpy $UNINST_REG_KEY "COH"
		ReadRegStr $R2 HKCU SOFTWARE\Cryptic\EUCOH "Installation Directory"
		; remove registry keys
		DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\$UNINST_REG_KEY"
		DeleteRegKey HKCU "Software\Cryptic\$UNINST_REG_KEY"
		DeleteRegKey /ifempty HKCU "Software\Cryptic"
		DeleteRegKey HKLM "SOFTWARE\Cryptic\$UNINST_REG_KEY\debug"
		DeleteRegKey HKLM "SOFTWARE\Cryptic\$UNINST_REG_KEY"
		DeleteRegKey /ifempty HKLM "Software\Cryptic"

		; did they install the english version over the european (leaving the registry keys)?
		StrCmp $R1 $R2 deletingEUCoh
	Goto doneDeletingRegistryKeys

	deletingEUCoh:
		StrCpy $UNINST_REG_KEY "EUCOH"
		; remove registry keys
		DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\$UNINST_REG_KEY"
		DeleteRegKey HKCU "Software\Cryptic\$UNINST_REG_KEY"
		DeleteRegKey /ifempty HKCU "Software\Cryptic"
		DeleteRegKey HKLM "SOFTWARE\Cryptic\$UNINST_REG_KEY\debug"
		DeleteRegKey HKLM "SOFTWARE\Cryptic\$UNINST_REG_KEY"
		DeleteRegKey /ifempty HKLM "Software\Cryptic"
	Goto doneDeletingRegistryKeys

	deletingKRCoh:
		StrCpy $UNINST_REG_KEY "KRCOH"
		; remove registry keys
		DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\$UNINST_REG_KEY"
		DeleteRegKey HKCU "Software\Cryptic\$UNINST_REG_KEY"
		DeleteRegKey /ifempty HKCU "Software\Cryptic"
		DeleteRegKey HKLM "SOFTWARE\Cryptic\$UNINST_REG_KEY\debug"
		DeleteRegKey HKLM "SOFTWARE\Cryptic\$UNINST_REG_KEY"
		DeleteRegKey /ifempty HKLM "Software\Cryptic"
	Goto doneDeletingRegistryKeys

	deletingCohTest:
		StrCpy $UNINST_REG_KEY "COHTest"
		; remove registry keys
		ReadRegStr $R2 HKCU SOFTWARE\Cryptic\EUCohTest "Installation Directory"
		DeleteRegKey HKCU "Software\Cryptic\$UNINST_REG_KEY"
		DeleteRegKey /ifempty HKCU "Software\Cryptic"
		DeleteRegKey /ifempty HKLM "Software\Cryptic"
		
		; did they install the english version over the european (leaving the registry keys)?
		StrCmp $R1 $R2 deletingEUCohTest
	Goto doneDeletingRegistryKeys

	deletingEUCohTest:
		StrCpy $UNINST_REG_KEY "EUCOHTest"
		; remove registry keys
		DeleteRegKey HKCU "Software\Cryptic\$UNINST_REG_KEY"
		DeleteRegKey /ifempty HKCU "Software\Cryptic"
		DeleteRegKey /ifempty HKLM "Software\Cryptic"
	Goto doneDeletingRegistryKeys

	doneDeletingRegistryKeys:
  
SectionEnd



;--------------------------------
;Uninstaller macros

; this has to be defined here in order to support certain changes to the MUI code
; without modifying MUI.nsh.  

!macro UNGETLANGUAGE

	Push $R0
	!ifdef MUI_LANGDLL_REGISTRY_ROOT & MUI_LANGDLL_REGISTRY_KEY & MUI_LANGDLL_REGISTRY_VALUENAME
	  
		ReadRegStr $R0 "${MUI_LANGDLL_REGISTRY_ROOT}" "${MUI_LANGDLL_REGISTRY_KEY}" "${MUI_LANGDLL_REGISTRY_VALUENAME}"
		StrCmp $R0 "" 0 mui.ungetlanguage_setlang
	  
	!endif
	    
	;!insertmacro MUI_LANGDLL_DISPLAY
	; for the uninstaller, if the language wasnt found in the registry, just default to english
	StrCpy $LANGUAGE 1033
	      
	!ifdef MUI_LANGDLL_REGISTRY_ROOT & MUI_LANGDLL_REGISTRY_KEY & MUI_LANGDLL_REGISTRY_VALUENAME
	  
		Goto mui.ungetlanguage_done
	   
		mui.ungetlanguage_setlang:
			StrCpy $LANGUAGE $R0
	        
		mui.ungetlanguage_done:

	!endif
	  
	Pop $R0
  
!macroend



;--------------------------------
;Uninstaller Functions

Function un.onInit

	;!insertmacro MUI_UNGETLANGUAGE
	!insertmacro UNGETLANGUAGE

	; StrCpy $NAME_TO_USE "City of Hereos / "
  
FunctionEnd


;--------------------------------
;Descriptions

	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
		!insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
		!insertmacro MUI_DESCRIPTION_TEXT ${SecDShortcuts} $(DESC_SecDShortcuts)
	!insertmacro MUI_FUNCTION_DESCRIPTION_END

