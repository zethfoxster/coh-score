
!include "${NSISDIR}\Contrib\System\System.nsh"


Name "BgImage.dll test"

OutFile "BgImage Test.exe"

XPStyle on

!define DEBUG
!macro GetReturnValue
!ifdef DEBUG
	Pop $R9
	StrCmp $R9 success +2
		DetailPrint "Error: $R9"
!endif
!macroend

;Function .onGUIInit
;
;	InitPluginsDir
;	BgImage::SetReturn /NOUNLOAD on
;
;	; Create RECT struct
;	System::Call "*${stRECT} .r1"
;
;	; Determine the screen work area
;	System::Call "User32::SystemParametersInfo(i, i, i, i) i (${SPI_GETWORKAREA}, 0, r1, 0) .r4" 
;	; Get left/top/right/bottom
;	System::Call "*$1${stRECT} (0, 0, 0, .r7)"
;;	MessageBox MB_OK "Got $4 $5 $6 $7"
;
;	System::Free $1
;
;
;
;		File /oname=$PLUGINSDIR\1.bmp "C:\Game\src\Installer\Resources\Blaster_NoText.bmp"
;		BgImage::SetBg /NOUNLOAD /FILLSCREEN $PLUGINSDIR\1.bmp
;		!insertmacro GetReturnValue
;
;	
;	# create the font for the following text
;		# Calc font size
;		# 60 @ 768
;		# 42 @ 600
;		IntOp $2 $7 * 18
;		IntOp $2 $2 / 168
;		IntOp $2 $2 - 22
;		CreateFont $R0 "Arial" $2 700 /ITALIC
;	# BLASTER shadow and text
;		BgImage::AddText /NOUNLOAD "BLASTER" $R0 0 0 0 54 34 800 200
;		!insertmacro GetReturnValue
;		BgImage::AddText /NOUNLOAD "BLASTER" $R0 255 16 16 50 30 800 200
;		!insertmacro GetReturnValue
;
;	# create the font for the following text
;		CreateFont $R0 "Arial" 14 700
;!define TEXT "The Blaster is an offensive juggernaut.  Though he can deal a lot of damange up close, he prefers to launch his powerful ranged attacks from a distance.  The Blaster has fewer Hit Points than other offensive heroes, and he can't stand toe to toe with most opponents for long.  His best defense is a great offense."
;	# Description shadow and text
;		BgImage::AddText /NOUNLOAD "${TEXT}" $R0 0 0 0		 -571 -155 -17 -1
;		!insertmacro GetReturnValue
;		BgImage::AddText /NOUNLOAD "${TEXT}" $R0 255 220 220 -574 -158 -20 -1
;		!insertmacro GetReturnValue
;	
;;		BgImage::Redraw /NOUNLOAD
;		BgImage::Redraw /NOUNLOAD
;	
;FunctionEnd
;
;ShowInstDetails show
;
;Section
;
;
;SectionEnd
;
;Function .onGUIEnd
;	# Destroy must not have /NOUNLOAD so NSIS will be able to unload
;	# and delete BgImage before it exits
;	BgImage::Destroy
;	# Destroy doesn't return any value
;FunctionEnd