;
;--------------------------------

!include "pathscripts.nsi"


; The name of the installer
Name "Cryptic Studios New System Setup"

; The file to write
OutFile "CrypticNewSystem.exe"

SetPluginUnload  alwaysoff

!include "ScheduleTask.nsi" ; Must be after SetPluginUnload, I guess

; The default installation directory
;InstallDir C:\Game\ // Need a comment, can't end on a backslash!

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
;InstallDirRegKey HKLM SOFTWARE\NSIS_Example2 "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will set up your system with some Cryptic Studios stuff"

; The text to prompt the user to enter a directory
;DirText "Choose a directory to install in to:"

InstType "Default"
InstType "Artist"
InstType "Programmer"
InstType "Programmer Refresh"
InstType "Programmer After VisualStudio"
InstType "None"

;--------------------------------

; The stuff to install
Section "Path Setup (c:\Cryptic\tools\bin)"

	SectionIn 1 2 3 4
	Push C:\Cryptic\tools\bin
	Call AddToPath  
  
SectionEnd


Section "Path Setup (c:\game\tools\util)"

; Not for FightClub people anymore
;	SectionIn 1 2 3
	Push C:\game\tools\util
	Call AddToPath  
  
SectionEnd

; Must be early, as it requires user interaction
Section "Edit Plus"
	SectionIn 1 2 3
	MessageBox MB_OK "The EditPlus installation will now launch, please click OK and accept all default installation paths"
	ExecWait '"N:\Software\Editplus\epp230_en.exe" /auto'
	SetOutPath "$PROGRAMFILES\EditPlus 2\"
	File tool.ini
	SetOutPath "$APPDATA\EditPlus 2\"
	File tool.ini
	File default0_mac
	File default1_mac
	File editplus.ini

	ExecWait 'regedit /s N:\Software\Editplus\License.reg'
SectionEnd

Section "VNC"
	SectionIn 1 2 3
	ExecWait '"N:\Software\VNC\vnc_ForCrypticNewSystem.exe" /silent'
	ExecWait 'regedit /s N:\Software\VNC\Options.reg'
SectionEnd

Section "Hosts file"

	SectionIn 1 2 3
	SetOutPath $SYSDIR\drivers\etc
	File C:\WINDOWS\SYSTEM32\DRIVERS\ETC\hosts

SectionEnd

Section "Gimme Registry hooks"
	SectionIn 1 2 3
	Exec "N:\revisions\gimme.exe -register"
SectionEnd

Section "Fix Windows Find"
	SectionIn 1 2 3 4
	WriteRegDWORD HKLM SYSTEM\CurrentControlSet\Control\ContentIndex FilterFilesWithUnknownExtensions 1
SectionEnd

; No longer desired
Section "VS2003 Remote Debugger"
	ExecWait 'msiexec /qb- /i "N:\Software\Visual Studio .NET 2003 Professional\vs_setup.msi" NOVSUI=1 TRANSFORMS="N:\Software\Visual Studio .NET 2003 Professional\Setup\Rmt9x.mst" SERVER_SETUP=1 ADDLOCAL=Native_Only_Debugging'
	SetShellVarContext All
	Delete "$SMPROGRAMS\Microsoft Visual Studio .NET 2003\Visual Studio .NET Tools\Visual C++ Remote Debugger.lnk"
	SetOutPath "$SMPROGRAMS\Microsoft Visual Studio .NET 2003\Visual Studio .NET Tools\"
	CreateShortCut "$SMPROGRAMS\Microsoft Visual Studio .NET 2003\Visual Studio .NET Tools\Visual C++ Remote Debugger.lnk" "C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\Packages\Debugger\msvcmon.exe" "/anyuser /tcpip"
SectionEnd

Section "Make Outlook less paranoid"
	SectionIn 3
	WriteRegExpandStr HKCU "Software\Microsoft\Office\11.0\Outlook\Security" Level1Remove ".zip;.exe;.msc;.reg;.bat;.url"
SectionEnd

Section "Show file extensions (requires logout)"

	SectionIn 1 2 3
	WriteRegDWORD HKCU Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced HideFileExt 0

SectionEnd

Section "Beyond Compare"
	SectionIn 1 2 3
	WriteRegExpandStr HKCU "Software\Scooter Software\Beyond Compare" CertKey "kxZoVDVfdxRwOuiWWzQYGgYSjEmunWStRJMB8P+qkNXMx3F8FcDrBnI9YULxVinUXYBco+RCtiJJ6MHR+sACOhoMAWlsEBvZ3d-674kwaXgzrUvMqEIKNFNGRgslit5mTocQYYqqG5KppaMeXu+FP0DcHAj1niYLePLXWavps6hcGTqJtkb+kBuZ4aLJdByinqNNgn0Flm0glDPyVR-Tomwkt6wEKKxIQOrQHSJkYlb1b-RxFnsvkr9vluMEYhHK"
	Exec "N:\Software\BeyondCompare2\beycomp_070308.exe /sp- /silent /norestart"
SectionEnd

Section "Cryptic Web Shortcuts"
	SectionIn 1 2 3
	CreateDirectory "$FAVORITES\Cryptic\"
	CreateShortCut "$FAVORITES\Cryptic\Confluence.lnk" "http://code:8081/display/cs/Home"
	CreateShortCut "$FAVORITES\Cryptic\JIRA.lnk" "http://code:8080/"
	CreateShortCut "$FAVORITES\Cryptic\Cryptic Internal Forums.lnk" "http://penguin3/forum/"
;	CreateShortCut "$FAVORITES\Cryptic\City of Heroes Official Forums.lnk" "https://boards.cityofheroes.com/ubbthreads.php"
;	CreateShortCut "$FAVORITES\Cryptic\Cryptic Studios Contact List.lnk" "N:\employee resources\contact.htm"
SectionEnd

Section "CoH: Get Latest Data, Src, and Executables"

;	Not for FightClub anymore
;	SectionIn 2
	ExecWait "n:\revisions\initial glv all.bat"

	AddSize 80000000 ; /src
	AddSize 20000000 ;/data
	AddSize 1000000 ; /tools
	AddSize 2000000 ; /docs
SectionEnd

Section "CoH: Get Latest Data and Executables"

;	Not for FightClub anymore
;	SectionIn 1 3

	ExecWait "n:\revisions\initial glv data tools.bat"

	AddSize 20000000 ;/data
	AddSize 1000000 ; /tools
	AddSize 2000000 ; /docs
SectionEnd

Section "CoH: Desktop Shortcuts"

	CreateShortCut "$DESKTOP\Game.lnk" "C:\Game\tools\util\CityOfHeroes.exe" "-console -cs test -can_edit -localmapserver -cryptic -notimeout"
	CreateShortCut "$DESKTOP\MapServer.lnk" "C:\Game\tools\util\MapServer.exe" "-db test" 
	CreateShortCut "$DESKTOP\Shortcuts.lnk" "C:\Game\tools\shortcuts\"

SectionEnd

Section "FileWatcher Config (disables CoH)"
	SectionIn 1 2 3 4
	SetOutPath "C:\"
	File filewatch.txt
SectionEnd

Section "FightClub: Get Latest Data, Src, and Executables"

	SectionIn 2
	ExecWait "n:\revisions\initial glv FC all + coh tools.bat"

	AddSize 80000000 ; /src
	AddSize 20000000 ;/data
	AddSize 1000000 ; /tools
	AddSize 2000000 ; /docs
SectionEnd

Section "FightClub: Get Latest Data and Executables"

	SectionIn 1 3

	ExecWait "n:\revisions\initial glv FC data tools + coh tools.bat"


	AddSize 20000000 ;/data
	AddSize 1000000 ; /tools
	AddSize 2000000 ; /docs
SectionEnd

Section "LostWorlds: Get Latest Data, Src, and Executables"

;	SectionIn 2
	ExecWait "n:\revisions\initial glv LW all.bat"

	AddSize 80000000 ; /src
	AddSize 20000000 ;/data
	AddSize 1000000 ; /tools
	AddSize 2000000 ; /docs
SectionEnd

Section "LostWorlds: Get Latest Data and Executables"

;	SectionIn 1 3

	ExecWait "n:\revisions\initial glv LW no src.bat"


	AddSize 20000000 ;/data
	AddSize 1000000 ; /tools
	AddSize 2000000 ; /docs
SectionEnd


SectionGroup "Programmer Stuff"

	Section "Symbol Server"
		SectionIn 3 4
		WriteRegExpandStr HKCU "Environment" _NT_SYMBOL_PATH srv*c:\src\symcache*N:\nobackup\symserv\data;srv*c:\src\symcache*N:\nobackup\symserv\dataVS8
		WriteRegExpandStr HKCU "Environment" _NT_IMAGE_PATH srv*c:\src\symcache*N:\nobackup\symserv\data;srv*c:\src\symcache*N:\nobackup\symserv\dataVS8
		WriteRegExpandStr HKCU "Software\Microsoft\VisualStudio\7.1\NativeDE\Dumps" GlobalModPath "srv*c:\src\symcache*N:\nobackup\symserv\data;srv*c:\src\symcache*N:\nobackup\symserv\dataVS8"
	SectionEnd

	Section "Subversion Config File"
		SectionIn 3
		SetOutPath "$APPDATA\Subversion"
		File "C:\Cryptic\tools\programmers\svn\config"
	SectionEnd

	Section "Nightly Source Code Backup"
		SectionIn 3 4

		SetOutPath $TEMP

		push "backup_source"
		push "Nightly Source Code Backup"
		push "C:\Cryptic\tools\programmers\nightly_backup\backup_source.bat"
		push "C:\Cryptic\tools\programmers\nightly_backup"
		push "NoProgramArgs"
		; 1 = DAILY  - field 15 gives interval in days (here it 1)
		push "*(&l2, &i2 0, &i2 2003, &i2 9, &i2 3, &i2 0, &i2 0, &i2 0, &i2 5, &i2 00, i 0, i 0, i 0, i 1, &i2 1, i 0, i 0, &i2 0) i.s"

		Call CreateTask
		Pop $0
		; MessageBox MB_OK "CreateTask result: $0"

		; last plugin call must not have /NOUNLOAD so NSIS will be able to delete the temporary DLL
		SetPluginUnload manual
		; do nothing
		System::Free 0

	SectionEnd

	Section "Tortoise SVN"
		SectionIn 3
		ExecWait 'msiexec /quiet /norestart /i "N:\Software\subversion\TortoiseSVN-ForCrypticNewSystem.msi"'
	SectionEnd

	Section "Set SVN Diff to BeyondCompare"
		SectionIn 3 4
		WriteRegStr HKCU "Software\TortoiseSVN" Diff "$PROGRAMFILES\Beyond Compare 2\BC2.EXE"
	SectionEnd

	; Must be after Tortoise SVN
	Section "Get C:\SRC"
		SectionIn 3
		ExecWait 'C:\Cryptic\tools\bin\svn checkout svn://code/dev C:\src --username brogers --password "" --no-auth-cache'
	SectionEnd

	Section "Visual Studio 2005 TeamSuite (may require reboots?)"
		SectionIn 3
		; Might not be working...
		; Might require reboot, so put LAST of the "SectionIn 3"s
		ExecWait '"N:\nobackup\MSDN Subscriptions\Visual Studio 2005 TeamSuite\vs\Setup\setup.exe" /unattendfile "N:\nobackup\MSDN Subscriptions\Visual Studio 2005 TeamSuite\vs\vs2005_deployment.ini"'

	SectionEnd

	Section "Wait for all installers to finish"
		SectionIn 3 5
		ExecWait "N:\bin\BatUtil.exe WaitForExit 2000 msiexec.exe setup.exe"
	SectionEnd

	Section "Visual Studio 2005 TeamSuite SP1 patch"
		SectionIn 5
		ExecWait '"N:\nobackup\MSDN Subscriptions\VS 2005 SP1\VS80sp1-KB926601-X86-ENU.exe" /passive /qn'
	SectionEnd

	Section "Wait for all installers to finish"
		SectionIn 5
		ExecWait "N:\bin\BatUtil.exe WaitForExit 2000 msiexec.exe setup.exe VS80sp1-KB926601-X86-ENU.exe"
	SectionEnd

	; Must be after Visual Studio 2005
	Section "IncrediBuild"
		SectionIn 5
		; No silent install? :(
		ExecWait '"N:\Software\IncrediBuild\IBForCrypticNewSystem.exe"'
		WriteRegStr HKLM "Software\Xoreax\IncrediBuild\Builder" Flags "All,Beep"
	SectionEnd

	; Must be after Visual Studio 2005
	Section "Visual Assist"
		SectionIn 5
		; No silent install? :(
		ExecWait '"N:\Software\VA_X_ForCrypticNewSystem.exe"'
	SectionEnd

	; Silent works
	; Must be after Visual Studio 2005
	Section "Workspace Whiz"
		SectionIn 5
		ExecWait '"N:\Software\WorkspaceWhiz\WorkspaceWhiz40VSNET_LatestCryptic.exe" /silent'
	SectionEnd

	; Silent works
	; Must be after Visual Studio 2005
	Section "Xbox XDK"
		SectionIn 5
		ExecWait 'N:\Software\xdk\installForCrypticNewSystem.bat'
	SectionEnd

	Section "VS8 SVN External Tools"
		SectionIn 5
		ExecWait 'regedit /s C:\Cryptic\tools\programmers\svn\SVNVisualStudio2005-32.reg'
	SectionEnd

	; Must be after Get C:\SRC
	Section "VS8 Team Settings"
		SectionIn 5
		ExecWait 'regedit /s C:\src\Utilities\VS8Settings\InstallSettings.reg'

		CopyFiles "$PROGRAMFILES\Microsoft Visual Studio 8\Common7\IDE\Profiles\General.vssettings" "$PROGRAMFILES\Microsoft Visual Studio 8\Common7\IDE\Profiles\General.vssettings.bak"
		CopyFiles "$PROGRAMFILES\Microsoft Visual Studio 8\Common7\IDE\Profiles\VC.vssettings" "$PROGRAMFILES\Microsoft Visual Studio 8\Common7\IDE\Profiles\General.vssettings"
		WriteRegStr HKCU "Software\Whole Tomato\Visual Assist X" ShowTipOfTheDay "No"
		ExecWait '"$PROGRAMFILES\Microsoft Visual Studio 8\Common7\IDE\devenv.exe" /Command Exit /ResetSettings "C:\src\Utilities\VS8Settings\ToolbarAndKeyboard.vssettings"'
		WriteRegStr HKCU "Software\Whole Tomato\Visual Assist X" ShowTipOfTheDay "Yes"
		CopyFiles "$PROGRAMFILES\Microsoft Visual Studio 8\Common7\IDE\Profiles\General.vssettings.bak" "$PROGRAMFILES\Microsoft Visual Studio 8\Common7\IDE\Profiles\General.vssettings"

	SectionEnd

	Section "VS8 Disable Macro Balloon"
		SectionIn 4 5
		WriteRegDWORD HKCU "Software\Microsoft\VisualStudio\8.0" DontShowMacrosBalloon 6
	SectionEnd


SectionGroupEnd