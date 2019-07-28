!define CD_NUMBER 2
!define CD_NUMBER2 2

!include "Coh.data.nsh"

Section "datafiles"

	Call repositionWindow

	System::Call "User32::SetForegroundWindow(i) b ($HWNDPARENT)"

  ; Stupid hackery in order to make the "second" installer's progress bar start at around half way
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4
;!insertmacro COPY_FILE_INFLATE_PROGRESS_NOSLEEP4

	!include "Coh.files${CD_NUMBER}.nsh"

SectionEnd