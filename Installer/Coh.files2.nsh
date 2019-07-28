; Included from Coh.data#.nsi and Coh.main.nsi

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_05
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_05
!else
	!ifdef COV_INSTALLER
		!insertmacro SPLASHIMAGE Vinstallimage_03
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE koreaninstallimage_03
		!else
			;!insertmacro SPLASHIMAGETEXT Defender $(SPLASH_Defender_Image)  $(SPLASH_Defender_Name) $(SPLASH_Defender_Text)  16 16 255  220 220 255  0 0 0
			!insertmacro SPLASHIMAGE Cityimage_03
		!endif
	!endif
!endif

SetOutPath $INSTDIR\piggs

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\sound2.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\sound3.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundPowers.pigg"


!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_06
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_06
!else
	!ifdef COV_INSTALLER
		!insertmacro SPLASHIMAGE Vinstallimage_04
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE koreaninstallimage_04
		!else
			;!insertmacro SPLASHIMAGETEXT Controller $(SPLASH_Controller_Image) $(SPLASH_Controller_Name) $(SPLASH_Controller_Text)  255 16 255  255 220 255  0 0 0
			!insertmacro SPLASHIMAGE Cityimage_04
		!endif
	!endif
!endif

; 244 MB

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texEnemies5.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texFxGuiItems.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texGui1.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_07
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_07
!endif

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texGui2.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texMaps.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_08
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_08
!else
	!ifdef COV_INSTALLER
		!insertmacro SPLASHIMAGE Vinstallimage_05
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE koreaninstallimage_05
		!else
			;!insertmacro SPLASHIMAGETEXT Scrapper $(SPLASH_Scrapper_Image) $(SPLASH_Scrapper_Name) $(SPLASH_Scrapper_Text)  255 255 16  255 255 220  0 0 0
			!insertmacro SPLASHIMAGE Cityimage_05
		!endif
	!endif
!endif

; 241 MB
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texMisc.pigg"
!insertmacro COPY_FILE_INFLATE_PROGRESS
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texPlayers2.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texVWorld.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_09
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_09
!endif

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texWorld.pigg"
!insertmacro COPY_FILE_INFLATE_PROGRESS
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texWorldSW.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texEnemies.pigg"