!define CD_NUMBER 1
!define CD_NUMBER1 1

!include "Coh.data.nsh"

Section "datafiles"

	Call repositionWindow

	System::Call "User32::SetForegroundWindow(i) b ($HWNDPARENT)"

	!include "Coh.files${CD_NUMBER}.nsh"

  ; Stupid hackery in order to make the "first" installer's progress bar stop at around half way
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4

SectionEnd