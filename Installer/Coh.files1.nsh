; Included from Coh.data#.nsi and Coh.main.nsi

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_01
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_01
!else
	!ifdef COV_INSTALLER
		!insertmacro SPLASHIMAGE Vinstallimage_01
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE koreaninstallimage_01
		!else
			;!insertmacro SPLASHIMAGETEXT Tanker $(SPLASH_Tanker_Image) $(SPLASH_Tanker_Name) $(SPLASH_Tanker_Text)  0 190 245  220 220 255  0 0 0
			!insertmacro SPLASHIMAGE Cityimage_01
		!endif
	!endif
!endif

SetOutPath $INSTDIR

; 9.2 MB
!insertmacro INSTALL_FILE_FATAL "${SOURCE_DIR}\CityOfHeroes.exe"

!ifdef COV_INSTALLER
	!ifdef ENGLISH_ONLY
		!insertmacro INSTALL_FILE_FATAL "C:\game\src\installer\V_Resources\CovUpdater.exe"
		!ifndef BETA_INSTALLER
			File /nonfatal "C:\game\src\installer\v_lang\README.*"
		!endif
	!else
		!ifdef KOREAN_ONLY
			!insertmacro INSTALL_FILE_FATAL "C:\game\src\installer\V_Resources\CovUpdater.KR.exe"
		!else
			!insertmacro INSTALL_FILE_FATAL "C:\game\src\installer\V_Resources\CovUpdater.EU.exe"
			File /nonfatal "C:\game\src\installer\v_lang\Readme-EN.*"
			File /nonfatal "C:\game\src\installer\v_lang\Readme-FR.*"
			File /nonfatal "C:\game\src\installer\v_lang\Readme-DE.*"
			File /nonfatal "C:\game\src\installer\v_lang\Readme-ES.*"
		!endif
	!endif
!else
	!ifdef GR_INSTALLER
		File "C:\game\src\installer\README*.*"
		!ifndef ENGLISH_ONLY
			File /nonfatal "C:\game\src\installer\gr_lang\README-*.*"
		!endif
	!else ifdef COMBO_INSTALLER
		File "C:\game\src\installer\README*.*"
		File /nonfatal "C:\game\src\installer\v_lang\README-*.*"
	!else
		!ifdef KOREAN_ONLY
			;File /nonfatal "C:\game\src\installer\lang\README.*"
		!else
			File "C:\game\src\installer\README*.*"
			File /nonfatal "C:\game\src\installer\lang\README*.*"
		!endif
	!endif
!endif

!insertmacro INSTALL_FILE_FATAL_NOMINI "${SOURCE_DIR}\cg.dll"
!insertmacro INSTALL_FILE_FATAL_NOMINI "${SOURCE_DIR}\cggl.dll"
!insertmacro INSTALL_FILE_FATAL_NOMINI "${SOURCE_DIR}\dfengine.dll"
!insertmacro INSTALL_FILE_FATAL_NOMINI "${SOURCE_DIR}\NxCooking.dll"
!insertmacro INSTALL_FILE_FATAL_NOMINI "${SOURCE_DIR}\PhysXLoader.dll"
!insertmacro INSTALL_FILE_FATAL_NOMINI "${SOURCE_DIR}\PhysXCore.dll"

!ifndef MINI
	File /oname=CoH.checksum "${CHECKSUM_FILE}"
!endif
!insertmacro COPY_FILE_INFLATE_PROGRESS
!insertmacro INSTALL_FILE_FATAL_NOMINI "${SOURCE_DIR}\DbgHelp.dll"
!insertmacro INSTALL_FILE_FATAL_NOMINI "${SOURCE_DIR}\CrashRpt.dll"

!ifdef MAJORPATCH_FILE
	File /oname=CoH.majorpatch "${MAJORPATCH_FILE}"
	!insertmacro COPY_FILE_INFLATE_PROGRESS
!endif

SetOutPath $INSTDIR\cider

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\cider\eula-en.html"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\cider\eula-eu-de.html"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\cider\eula-eu-en.html"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\cider\eula-eu-fr.html"

SetOutPath $INSTDIR\doublefusion
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\engine.var"

SetOutPath $INSTDIR\doublefusion\cache
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\objects.dfo"

; These are all the default textures instead of ads. They have to be here because
; the update server will patch to them if they aren't already installed.
SetOutPath $INSTDIR\doublefusion\cache\data\generic_tech_wall_128_512
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_128_512\df.dat"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_128_512\generic_tech_wall_128_512.tga"
SetOutPath $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_128
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_256_128\df.dat"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_256_128\generic_tech_wall_256_128.tga"
SetOutPath $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_256
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_256_256\df.dat"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_256_256\generic_tech_wall_256_256.tga"
SetOutPath $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_512
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_256_512\df.dat"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_256_512\generic_tech_wall_256_512.tga"
SetOutPath $INSTDIR\doublefusion\cache\data\generic_tech_wall_256_1024
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_256_1024\df.dat"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_256_1024\generic_tech_wall_256_1024.tga"
SetOutPath $INSTDIR\doublefusion\cache\data\generic_tech_wall_512_128
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_512_128\df.dat"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_512_128\generic_tech_wall_512_128.tga"
SetOutPath $INSTDIR\doublefusion\cache\data\generic_tech_wall_512_256
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_512_256\df.dat"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_512_256\generic_tech_wall_512_256.tga"
SetOutPath $INSTDIR\doublefusion\cache\data\generic_tech_wall_512_512
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_512_512\df.dat"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_512_512\generic_tech_wall_512_512.tga"
SetOutPath $INSTDIR\doublefusion\cache\data\generic_tech_wall_1024_256
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_1024_256\df.dat"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\doublefusion\cache\data\generic_tech_wall_1024_256\generic_tech_wall_1024_256.tga"

SetOutPath $INSTDIR\piggs

; 198 MB
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\sound1.pigg"
;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\sound.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundMusic.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundMusic1.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_02
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_02
!endif

;!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundV1.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundV3.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\soundV4.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_03
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_03
!else
	!ifdef COV_INSTALLER
		!insertmacro SPLASHIMAGE Vinstallimage_02
	!else
		!ifdef KOREAN_ONLY
			!insertmacro SPLASHIMAGE koreaninstallimage_02
		!else
			;!insertmacro SPLASHIMAGETEXT Blaster $(SPLASH_Blaster_Image) $(SPLASH_Blaster_Name) $(SPLASH_Blaster_Text) 255 16 16  255 220 220  35 20 40
			!insertmacro SPLASHIMAGE Cityimage_02
		!endif
	!endif
!endif 

; 232 MB
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texEnemies2.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texVEnemies.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texWorldBC.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texWorldCZ.pigg"

!ifdef GR_INSTALLER
	!insertmacro SPLASHIMAGE GR_Screenshot_04
!else ifdef COMBO_INSTALLER
	!insertmacro SPLASHIMAGE ComboInstall_04
!endif

!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texWorldV1.pigg"
!insertmacro INSTALL_FILE_NOMINI "${SOURCE_DIR}\piggs\texWorldV2.pigg"

# for best buy combo
!ifdef BESTBUY_INSTALLER
   !insertmacro INSTALL_FILE_FATAL "C:\game\src\installer\Best_Buy.ico"
!endif
