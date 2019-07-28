!define CD_NUMBER 3
!define CD_NUMBER3 3

!include "Coh.data.nsh"

Section "datafiles"

	Call repositionWindow

	System::Call "User32::SetForegroundWindow(i) b ($HWNDPARENT)"

	!include "Coh.files${CD_NUMBER}.nsh"
	
SectionEnd