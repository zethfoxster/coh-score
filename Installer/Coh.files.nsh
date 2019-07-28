
!macro INSTALL_FILE_FATAL FILE_NAME
	File "${FILE_NAME}"
	!insertmacro COPY_FILE_INFLATE_PROGRESS
!macroend

!macro INSTALL_FILE FILE_NAME
	File /nonfatal "${FILE_NAME}"
	!insertmacro COPY_FILE_INFLATE_PROGRESS
!macroend

!macro INSTALL_FILE_FATAL_NOMINI FILE_NAME
	!ifndef MINI
		File "${FILE_NAME}"
	!endif
	!insertmacro COPY_FILE_INFLATE_PROGRESS
!macroend

!macro INSTALL_FILE_NOMINI FILE_NAME
	!ifndef MINI
		File /nonfatal "${FILE_NAME}"
	!endif
	!insertmacro COPY_FILE_INFLATE_PROGRESS
!macroend



Function repositionWindowForSingleFile

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

	; X + 5, Y - 5
	; Left side of screen + 20
	IntOp $0 $4 + 5

	; Bottom of screen - window - 20
	IntOp $1 $7 - $3 ; Bottom of screen - window size
	IntOp $1 $1 - 5

	System::Call "User32::SetWindowPos(i, i, i, i, i, i, i) b ($HWNDPARENT, 0, $0, $1, 0, 0, 0x201)"

FunctionEnd



Function centerWindow

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

	; x0 + (x1 - x0 - width ) / 2
	; Center X
	IntOp $0 $6 - $4
	IntOp $0 $0 - $2
	IntOp $0 $0 / 2
	IntOp $0 $0 + $4

	; y0 + (y1 - y0 - height) / 2
	IntOp $1 $7 - $5
	IntOp $1 $1 - $3
	IntOp $1 $1 / 2
	IntOp $1 $1 + $5

	System::Call "User32::SetWindowPos(i, i, i, i, i, i, i) b ($HWNDPARENT, 0, $0, $1, 0, 0, 0x201)"

FunctionEnd