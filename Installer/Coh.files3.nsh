; Included from Coh.data#.nsi and Coh.main.nsi

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_10
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_10
!else
	!ifdef COV_INSTALLER
		!insertmacro SPLASHIMAGE Vinstallimage_06
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE koreaninstallimage_06
		!else
			!insertmacro SPLASHIMAGE Cityimage_06
		!endif	
	!endif
!endif

SetOutPath $INSTDIR\piggs

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\fonts.pigg"    ; 20
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\mapsCities1.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\mapsCities2.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\mapsMissions.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_11
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_11
!endif

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\mapsTrials.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\sound4.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\sound5.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\sound6.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundMusic.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_12
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_12
!else
	!ifdef COV_INSTALLER
		!insertmacro SPLASHIMAGE Vinstallimage_07
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE koreaninstallimage_07
		!else
			!insertmacro SPLASHIMAGE Cityimage_07
		!endif
	!endif
!endif

;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundV2.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundV5.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundV6.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundVMusic1.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundVMusic2.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundVMusic3.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_13
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_13
!endif

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundVMusic4.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundVMusic5.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundVMusic6.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundVMusic7.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_14
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_14
!else
	!ifdef COV_INSTALLER
		!insertmacro SPLASHIMAGE Vinstallimage_08
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE koreaninstallimage_08
		!else
			!insertmacro SPLASHIMAGE Cityimage_08
		!endif
	!endif
!endif

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundVMusic.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texVWorld1.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texVWorld2.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_15
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_15
!endif

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texVWorld3.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texVWorldBases1.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texVWorldBases2.pigg"