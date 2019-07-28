
;--------------------------------
;Language Selection Dialog Settings (must be before !insertmacro MUI_LANGDLL_DISPLAY )

!ifndef MUI_INCLUDED
	!include "MUILanguageSupport.nsh"
!endif

;--------------------------------
;Languages Part 1 of 2 (MUI default strings)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!ifdef KOREAN_ONLY | MAKE_UNINSTALLER
	!insertmacro MUI_LANGUAGE "Korean"
!endif
!ifdef DEBUG
  ;!insertmacro MUI_LANGUAGE "LEET"
!endif

!ifdef NOT_USED
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Japanese"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "Danish"
  !insertmacro MUI_LANGUAGE "Swedish"
  !insertmacro MUI_LANGUAGE "Norwegian"
;  !insertmacro MUI_LANGUAGE "Finnish"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Portuguese"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Ukrainian"
  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Slovak"
  !insertmacro MUI_LANGUAGE "Croatian"
;  !insertmacro MUI_LANGUAGE "Bulgarian"
  !insertmacro MUI_LANGUAGE "Hungarian"
;  !insertmacro MUI_LANGUAGE "Thai"
  !insertmacro MUI_LANGUAGE "Romanian"
  !insertmacro MUI_LANGUAGE "Latvian"
  !insertmacro MUI_LANGUAGE "Macedonian"
  !insertmacro MUI_LANGUAGE "Estonian"
  !insertmacro MUI_LANGUAGE "Turkish"
  !insertmacro MUI_LANGUAGE "Lithuanian"
  !insertmacro MUI_LANGUAGE "Catalan"
  !insertmacro MUI_LANGUAGE "Slovenian"
  !insertmacro MUI_LANGUAGE "Serbian"
  !insertmacro MUI_LANGUAGE "Arabic"
  !insertmacro MUI_LANGUAGE "Farsi"
  !insertmacro MUI_LANGUAGE "Hebrew"
  !insertmacro MUI_LANGUAGE "Indonesian"
!endif


;--------------------------------
;Reserve Files
  
  ;These files should be inserted before other files in the data block
  ;Keep these lines before any File command
  ;Only for solid compression (by default, solid compression is enabled for BZIP2 and LZMA)
  
  !insertmacro MUI_RESERVEFILE_LANGDLL
  
  

;--------------------------------
;Macro for grabbing the language chosen once
!macro INHERIT_LANGUAGE_FROM_WRAPPER
	Function .onInit
		!ifdef CD_NUMBER
			Call GetParameters
			  
			Pop $R0
			
			StrCmp $R0 "SECRET" SecretParam
			StrCmp $R0 "/SECRET" SecretParam NoSecret
			  
			SecretParam:
				SectionGetSize 0 $1
				FileOpen $2 "DataSize${CD_NUMBER}.nsh" w
				FileWrite $2 "AddSize $1$\r$\n"
				FileClose $2
				Quit
			NoSecret:
		!endif

		; Try to get value stored in registry from coh.wrapper.nsi
		ReadRegStr $R0 "${MUI_LANGDLL_REGISTRY_ROOT}" "${MUI_LANGDLL_REGISTRY_KEY}" "${MUI_LANGDLL_REGISTRY_VALUENAME} Temp"
		StrCmp $R0 "" langdll_show
			StrCpy $LANGUAGE $R0
			Goto langdll_done
		langdll_show:
			!ifdef SINGLE_FILE
				!ifdef ENGLISH_ONLY
				!else
					!ifdef KOREAN_ONLY
						StrCpy $LANGUAGE "1042"
						; DO NOT CHECK THIS IN uncommented:
						;StrCpy $LANGUAGE "1033"
					!else
						!insertmacro MUI_LANGDLL_DISPLAY
					!endif
				!endif
			!else
				!insertmacro MUI_LANGDLL_DISPLAY
			!endif

		langdll_done:
	  
	FunctionEnd
!macroend


;--------------------------------
; Languages Part 2 of 2
  !include "c:\game\src\installer\lang\English.nsh"
  !include "c:\game\src\installer\lang\French.nsh"
  !include "c:\game\src\installer\lang\German.nsh"
!ifdef KOREAN_ONLY | MAKE_UNINSTALLER
  !include "c:\game\src\installer\lang\Korean.nsh"
!endif
!ifdef DEBUG
  ;!include "c:\game\src\installer\lang\L337.nsh"
!endif

