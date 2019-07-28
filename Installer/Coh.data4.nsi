!define CD_NUMBER 4
!define CD_NUMBER4 4

!include "Coh.data.nsh"

Section "datafiles"

	Call repositionWindow

	System::Call "User32::SetForegroundWindow(i) b ($HWNDPARENT)"

	!include "Coh.files${CD_NUMBER}.nsh"

SectionEnd