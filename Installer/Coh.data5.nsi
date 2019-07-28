!define CD_NUMBER 5
!define CD_NUMBER5 5

!include "Coh.data.nsh"

Section "datafiles"

	Call repositionWindow

	System::Call "User32::SetForegroundWindow(i) b ($HWNDPARENT)"

	!include "Coh.files${CD_NUMBER}.nsh"

SectionEnd