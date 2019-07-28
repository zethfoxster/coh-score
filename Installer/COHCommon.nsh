;SetCompressor lzma

; for the bestbuy installer
; !define BESTBUY_INSTALLER

; DEBUG flag currently does:
;   Checks return values of BgImage calls
;   Does *not* write registry valuse
;	Set this flag in Version.nsh and our patch process will ignore it/override it
;   Does not check "already running" mutex
;!define DEBUG

; DEBUG_NODELAY
;   Does not add delays to simulate the copying of files (causes quicker build)
;!define DEBUG_NODELAY

; MINI flag currently does:
;   Copies only a small number of files
;   Adds a artificial delay for any files that would be copied but aren't in MINI
;	Set this flag in Version.nsh and our patch process will ignore it/override it
;!define MINI

; DISABLE_BGIMAGE flag:
;   Does not make any calls to BgImage
;!define DISABLE_BGIMAGE

; GR_INSTALLER makes the installer use GR files and art
!define GR_INSTALLER

; COV_INSTALLER makes the installer use COV files and art
;!define COV_INSTALLER

; COMBO_INSTALLER makes the installer use art for the CoH/V combo box
!define COMBO_INSTALLER

; BETA_INSTALLER is used to distinguish beta specific things
;!define BETA_INSTALLER

; GR_BETA_INSTALLER is used to distinguish GR-beta specific things
;!define GR_BETA_INSTALLER

; ENGLISH_ONLY disables asking about what language to use (defaults to English)
!define ENGLISH_ONLY

; turning this off makes the installer not build an uninstaller, and instead use the
; one in the patch.  MUST NOT BE ON FOR RELEASED INSTALLERS (causes mismatched uninstall.exe)
;  Build this then run Release/MakeUnInstaller.exe and have it make an uninstaller
;!define MAKE_UNINSTALLER

; set this to build just a bootstrap installer that includes the updater
;!define MAKE_UPDATER_ONLY

; SINGLE_FILE builds a large, single file which installs the game, with no temporary space used
;!define SINGLE_FILE

;!define DVD_VERSION
;!define KOREAN_ONLY
;!define KOREAN_OBT


;!define CD_COUNT_2
;!define CD_COUNT_3
!define CD_COUNT_4
;!define CD_COUNT_5

!ifdef CD_COUNT_5
	!define NUM_CDS 5
!else
	!ifdef CD_COUNT_4
		!define NUM_CDS 4
	!else
		!ifdef CD_COUNT_3
			!define NUM_CDS 3
		!else
			!define NUM_CDS 2
		!endif
	!endif
!endif

!ifdef MAKE_UNINSTALLER
	!ifndef MINI
		!define MINI
	!endif
;	!ifndef COV_INSTALLER
;		!define COV_INSTALLER
;	!endif
	!ifndef DEBUG_NODELAY
		!define DEBUG_NODELAY
	!endif
	!ifndef SINGLE_FILE
		!define SINGLE_FILE
	!endif
	!ifdef ENGLISH_ONLY
		!undef ENGLISH_ONLY
	!endif
	!ifdef KOREAN_ONLY
		!undef KOREAN_ONLY
	!endif
!endif

!ifdef MAKE_UPDATER_ONLY
	!ifndef MINI
		!define MINI
	!endif
;	!ifndef COV_INSTALLER
;		!define COV_INSTALLER
;	!endif
	!ifndef DEBUG_NODELAY
		!define DEBUG_NODELAY
	!endif
	!ifndef SINGLE_FILE
		!define SINGLE_FILE
	!endif
!endif
	

;--------------------------------
; Updater
;--------------------------------
!ifdef ENGLISH_ONLY
	; US Updater
	!ifdef GR_BETA_INSTALLER
		!define UPDATER_OUTPUT_NAME CohUpdater.exe
		!define GAME_REGISTRY_KEY COHBETA
	!else ifdef BETA_INSTALLER
		!define UPDATER_OUTPUT_NAME CoVBetaUpdater.exe
		!define GAME_REGISTRY_KEY COHTEST
	!else
		!define UPDATER_OUTPUT_NAME CohUpdater.exe
		!define GAME_REGISTRY_KEY COH
	!endif
!else
	!ifdef KOREAN_ONLY
		!define UPDATER_OUTPUT_NAME CohUpdater.KR.exe
		!define GAME_REGISTRY_KEY KRCOH
	!else
		; EU Updater
		!define UPDATER_OUTPUT_NAME CohUpdater.EU.exe
		!ifdef GR_BETA_INSTALLER
			!define GAME_REGISTRY_KEY EUCohTest
		!else
			!define GAME_REGISTRY_KEY EUCOH
		!endif
	!endif
!endif

!ifdef COV_INSTALLER
	; CoV Updater
	!ifdef ENGLISH_ONLY
		!define COV_UPDATER_OUTPUT_NAME CovUpdater.exe
	!else
		!ifdef KOREAN_ONLY
			!define COV_UPDATER_OUTPUT_NAME CovUpdater.KR.exe
		!else
			; EU Updater
			!define COV_UPDATER_OUTPUT_NAME CovUpdater.EU.exe
		!endif
	!endif
!endif

;--------------------------------
; Common configuration settings
;--------------------------------
	!include Version.nsh
	!ifdef COV_INSTALLER
		!define ENGLISH_NAME "City of Villains"
	!else
		!define ENGLISH_NAME "City of Heroes"
	!endif

	; The name of the installer
	Name "City of Heroes"
;	!ifdef COMBO_INSTALLER
;		Name "City of Heroes/City of Villains"
;	!else
;		Name "UNTRANSLATED NAME"
;	!endif
	
	VIAddVersionKey "FileVersion" "${VERSION}"
	VIAddVersionKey "FileDescription" "${ENGLISH_NAME}"
	VIAddVersionKey "ProductName" "${ENGLISH_NAME}"
	VIAddVersionKey "LegalCopyright" "Copyright (c) 1998-2004 Cryptic Studios"
	VIProductVersion "1.0.0.0"

	XPStyle On

	CRCCheck OFF ; this is really slow!

	BrandingText "Nullsoft Install System"
	
!ifdef CD_NUMBER
	!ifdef DVD_VERSION
		Caption "$(TITLE_InstallDiscDVD)"
	!else ifdef CD_NUMBER1
		Caption "$(TITLE_InstallDisc1)"
	!else ifdef CD_NUMBER2
		Caption "$(TITLE_InstallDisc2)"
	!else ifdef CD_NUMBER3
		Caption "$(TITLE_InstallDisc3)"
	!else ifdef CD_NUMBER4
		Caption "$(TITLE_InstallDisc4)"
	!else ifdef CD_NUMBER5
		Caption "$(TITLE_InstallDisc5)"
	!endif

!else
	!ifdef KOREAN_OBT
		Caption "$(TITLE_Setup_2)"
	!else
		Caption "$(TITLE_Setup)"
	!endif
!endif
	
	!define DATA_FILE_BASENAME CohData
	!ifdef COV_INSTALLER
		!define ICON_FILE "C:\Game\src\Installer\Resources\Icon_Cov.ico"
	!else
		!ifdef KOREAN_ONLY
			!define ICON_FILE "C:\Game\src\Installer\Resources\Icon_Korean.ico"
		!else
			!define ICON_FILE "C:\Game\src\Installer\Resources\Icon_Statesman.ico"
		!endif
	!endif
	!define MUI_ICON "${ICON_FILE}"
	!define MUI_UNICON "${ICON_FILE}"
	Icon "${ICON_FILE}"
	UninstallIcon "${ICON_FILE}"

	FileErrorText $(ERROR_FileErrorText)


  ;Remember the installer language (must be before !insertmacro MUI_PAGE_INSTFILES )
  !define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
  !define MUI_LANGDLL_REGISTRY_KEY "SOFTWARE\Cryptic\${GAME_REGISTRY_KEY}"
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"
	

;--------------------------------
;Macros
;--------------------------------

!macro GetReturnValue
!ifdef DEBUG
	Pop $R9
	StrCmp $R9 success +2
		DetailPrint "Error: $R9"
!endif
!macroend

; Puts the screen height in $2
!macro GETSCREENHEIGHT
	Push $1
	
	; Create RECT struct
	System::Call "*${stRECT} .r1"

	; Determine the screen work area
	System::Call "User32::SystemParametersInfo(i, i, i, i) i (${SPI_GETWORKAREA}, 0, r1, 0) 0" 
	; Get left/top/right/bottom
	System::Call "*$1${stRECT} (0, 0, 0, .r2)"

	System::Free $1
	
	Pop $1
!macroend

!macro SPLASHIMAGETEXT IMAGEBASE IMAGETYPE TITLE TEXT TITLER TITLEG TITLEB TEXTR TEXTG TEXTB TITLEdsR TITLEdsG TITLEdsB
	BgImage::Clear /NOUNLOAD
	StrCmp ${IMAGETYPE} "NoText" NoTextImage_${IMAGEBASE} TextImage_${IMAGEBASE}
	
	TextImage_${IMAGEBASE}:
		
		StrCmp ${IMAGETYPE} "English" 0 TextImageNoEnglish_${IMAGEBASE}
		!insertmacro SPLASHIMAGE ${IMAGEBASE}
		goto EndImage_${IMAGEBASE}
		
		TextImageNoEnglish_${IMAGEBASE}:
		;!insertmacro SPLASHIMAGE ${IMAGEBASE}
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE ${IMAGEBASE}_Korean
		!endif
		goto EndImage_${IMAGEBASE}
		
	NoTextImage_${IMAGEBASE}:

		; Add in the text overlay
		!insertmacro GETSCREENHEIGHT
		; Calc font size
		;	60 @ 768
		;	42 @ 600
		IntOp $2 $2 * 18
		IntOp $2 $2 / 168
		IntOp $2 $2 - 22
		CreateFont $R0 "Arial" $2 700 /ITALIC

		; Title text
			BgImage::AddText /NOUNLOAD ${TITLE} $R0 ${TITLEdsR} ${TITLEdsG} ${TITLEdsB} 55 35 -10 200
			!insertmacro GetReturnValue
			; Outline
			 BgImage::AddText /NOUNLOAD ${TITLE} $R0 255 255 255  49 29 -10 200
			 !insertmacro GetReturnValue
			 BgImage::AddText /NOUNLOAD ${TITLE} $R0 255 255 255  51 29 -10 200
			 !insertmacro GetReturnValue
			 BgImage::AddText /NOUNLOAD ${TITLE} $R0 255 255 255  49 31 -10 200
			 !insertmacro GetReturnValue
			 BgImage::AddText /NOUNLOAD ${TITLE} $R0 255 255 255  51 31 -10 200
			 !insertmacro GetReturnValue
			BgImage::AddText /NOUNLOAD ${TITLE} $R0 ${TITLER} ${TITLEG} ${TITLEB} 50 30 -10 200
			!insertmacro GetReturnValue
		; Description text
			CreateFont $R0 "Arial" 14 700
			; Outline
			 BgImage::AddText /NOUNLOAD "${TEXT}" $R0 0 0 0		 -573 -247 -19 -1
			 !insertmacro GetReturnValue
			 BgImage::AddText /NOUNLOAD "${TEXT}" $R0 0 0 0		 -573 -249 -19 -1
			 !insertmacro GetReturnValue
			 BgImage::AddText /NOUNLOAD "${TEXT}" $R0 0 0 0		 -575 -247 -21 -1
			 !insertmacro GetReturnValue
			 BgImage::AddText /NOUNLOAD "${TEXT}" $R0 0 0 0		 -575 -249 -21 -1
			 !insertmacro GetReturnValue
			BgImage::AddText /NOUNLOAD "${TEXT}" $R0 ${TEXTR} ${TEXTG} ${TEXTB} -574 -248 -20 -1
			!insertmacro GetReturnValue

		; Must be after text in order to redraw
		!insertmacro SPLASHIMAGE ${IMAGEBASE}_NoText
		goto EndImage_${IMAGEBASE}
		
	EndImage_${IMAGEBASE}:

	!ifdef DEBUG
		!ifndef DEBUG_NODELAY
			Sleep 5000
		!endif
	!endif

!macroend


!macro SPLASHIMAGE IMAGENAME
	!ifndef DISABLE_BGIMAGE
	!ifdef GR_INSTALLER
		File /oname=$PLUGINSDIR\${IMAGENAME}.bmp "C:\Game\Src\Installer\GR_Resources\${IMAGENAME}.bmp"
	!else ifdef COV_INSTALLER
		File /oname=$PLUGINSDIR\${IMAGENAME}.bmp "C:\Game\Src\Installer\V_Resources\${IMAGENAME}.bmp"
	!else
		File /oname=$PLUGINSDIR\${IMAGENAME}.bmp "C:\Game\Src\Installer\Resources\${IMAGENAME}.bmp"
	!endif
		BgImage::SetBg /NOUNLOAD /FILLSCREEN $PLUGINSDIR\${IMAGENAME}.bmp
		!insertmacro GetReturnValue

		BgImage::Redraw /NOUNLOAD
	!endif ; DISABLE_BGIMAGE
	!ifdef DEBUG
		!ifndef DEBUG_NODELAY
			Sleep 200
		!endif
	!endif
!macroend

!macro SPLASHIMAGEGRADIENT
	!ifndef DISABLE_BGIMAGE
		BgImage::SetBg /NOUNLOAD /GRADIENT 0x30 0x30 0x90 0x0 0x0 0x0
		!insertmacro GetReturnValue
	!endif
!macroend



; A hackery of macros to add dummy "no-op" instructions to the installer so that
;  it's progress bar appears relative to number of files copied (as opposed
;  to number of NSIS instructions as is the default behavior)

!macro INNER2
  Push $0
  Pop $0
!macroend
!macro INNER4
  !insertmacro INNER2
  !insertmacro INNER2
!macroend
!macro INNER8
  !insertmacro INNER4
  !insertmacro INNER4
!macroend
!macro INNER16
  !insertmacro INNER8
  !insertmacro INNER8
!macroend
!macro INNER32
  !insertmacro INNER16
  !insertmacro INNER16
!macroend
!macro COPY_FILE_INFLATE_PROGRESS
  !ifndef DEBUG_NODELAY
    !insertmacro INNER32
    !insertmacro INNER32
    !ifdef MINI
      DetailPrint "Debug copy file..."
      Sleep 250
    !endif
  !endif
!macroend
!macro COPY_FILE_INFLATE_PROGRESS_NOSLEEP
  !insertmacro INNER32
  !insertmacro INNER32
!macroend
!macro COPY_FILE_INFLATE_PROGRESS_NOSLEEP2
  !insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP
  !insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP
!macroend
!macro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
  !ifndef DEBUG_NODELAY
    !insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP2
    !insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP2
  !endif
!macroend
