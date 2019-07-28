; Included from Coh.data#.nsi and Coh.main.nsi

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_16
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_16
!else
	!ifdef COV_INSTALLER
		!insertmacro SPLASHIMAGE Vinstallimage_09
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE koreaninstallimage_09
		!else
			!insertmacro SPLASHIMAGE Cityimage_09
		!endif
	!endif
!endif

SetOutPath $INSTDIR\piggs

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\mapsHazards.pigg"   ; 7.4
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texts.pigg"	;5.1
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\geolm.pigg"
!insertmacro COPY_FILE_INFLATE_PROGRESS
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\geolmBC.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\geolmV1.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\geolmV2.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\geomBC.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_17
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_17
!endif



;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_Outdoor_Missions.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_unique_missions.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_V_City_00_01.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_V_City_01_01.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_V_City_01_02.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_V_City_02_01.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_V_City_03_01.pigg"


;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_V_City_03_02.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_V_City_04_01.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_V_PvP_02_01.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_V_PvP_03_01.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_V_PvP_04_01.pigg"

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texWorldBuildings.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texEnemies3.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texEnemies4.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texPlayersUI.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_18
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_18
!else
	!ifdef COV_INSTALLER
		!insertmacro SPLASHIMAGE Vinstallimage_10
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE koreaninstallimage_10
		!else
			!insertmacro SPLASHIMAGE Cityimage_10
		!endif
	!endif
!endif

!insertmacro COPY_FILE_INFLATE_PROGRESS

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundV1.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\geom.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\sound6.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\geolmBC.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_19
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_19
!endif

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\sound.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\player.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundV2.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\lightmaps_V_City_03_01.pigg"

; 231 MB
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\bin.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\geom.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\geomV1.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\geomV2.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_20
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_20
!else
	!ifdef KOREAN_ONLY
		!insertmacro SPLASHIMAGE koreaninstallimage_11
	!endif
!endif

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\mapsMisc.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\misc.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\player.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\playerFemHuge.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\playerMale.pigg"
