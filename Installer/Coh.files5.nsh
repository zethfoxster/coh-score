; Included from Coh.data#.nsi and Coh.main.nsi

!ifdef COV_INSTALLER
; JE: Changed this to not use one with English text at the request of Europe
	!ifdef ENGLISH_ONLY
		!insertmacro SPLASHIMAGE Vinstallimage_11
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE koreaninstallimage_12
		!else
			!insertmacro SPLASHIMAGE Vinstallimage_04
		!endif
	!endif
!else
	!insertmacro SPLASHIMAGE Cityimage_11
!endif

SetOutPath $INSTDIR\piggs