;Function FindFileInRoot
; Usage: Push on the filename to search for
; Return: Pop off a string, it will be "NOT FOUND" or a full path to the file
;Function FindFileInRoot

; Example:
;Function FindFileInRootExample
;	Push "disc2.diz"
;	Call FindFileInRoot
;	Pop $0
;	MessageBox MB_OK "Result : $0"
;FunctionEnd



Function DetectDrives

	Exch
	Exch $R0
	Exch
	Exch $R1
	Push $R2
	Push $R3
	Push $R4
	Push $R5
	Push $0
	Push $1
	StrCpy $R4 $R1

	# Drives Conversion

	StrCmp $R0 "All Drives" 0 +2
	StrCpy $R0 "Hard Drives, CDROM Drives, Diskette Drives, RAM Drives, Network	Drives"

	StrCmp $R0 "All Local Drives" 0 +2
	StrCpy $R0 "Hard Drives, CDROM Drives, Diskette Drives, RAM Drives"
	  
	StrCmp $R0 "Basic Drives" 0 +2
	StrCpy $R0 "Hard Drives, CDROM Drives, Diskette Drives"

	StrCmp $R0 "Disk Needed Drives" 0 +2
	StrCpy $R0 "CDROM Drives, Diskette Drives"

	# Memory for paths
	 
	System::Alloc 1024 
	Pop $R1 

	# Get drives 

	System::Call 'kernel32::GetLogicalDriveStringsA(i, i) i(1024, R1)' 

	enumok: 

	# One more drive?

	System::Call 'kernel32::lstrlenA(t) i(i R1) .R2' 
	IntCmp $R2 0 enumex 

	# Now, search for drives according to user conditions

	System::Call 'kernel32::GetDriveTypeA(t) i (i R1) .R3' 

	StrCpy $2 0

	Search:

	IntOp $2 $2 + 1

	StrCmp $2 1 0 +3
	StrCpy $0 "Hard Drives"
	StrCpy $1 3

	StrCmp $2 2 0 +3
	StrCpy $0 "CDROM Drives"
	StrCpy $1 5
	  
	StrCmp $2 3 0 +3
	StrCpy $0 "Diskette Drives"
	StrCpy $1 2
	  
	StrCmp $2 4 0 +3
	StrCpy $0 "RAM Drives"
	StrCpy $1 6
	  
	StrCmp $2 5 0 +3
	StrCpy $0 "Network Drives"
	StrCpy $1 4
	  
	IntCmp $2 6 enumnext 0 enumnext

	Push $R1
	Push $R2
	Push $R3
	Push $R4
	Push $R5

	StrLen $R3 $0
	StrCpy $R4 0
	loop:
		StrCpy $R5 $R0 $R3 $R4
		StrCmp $R5 $0 done
		StrCmp $R5 "" done
		IntOp $R4 $R4 + 1
		Goto loop
	done:
	StrCpy $R1 $R0 "" $R4
	   
	StrCmp $R1 "" 0 +7
	   
		Pop $R5
		Pop $R4
		Pop $R3
		Pop $R2
		Pop $R1
		Goto Search
	     
	Pop $R5
	Pop $R4
	Pop $R3
	Pop $R2
	Pop $R1
	   
	StrCmp $R3 $1 0 Search

	# Get drive path string 

	System::Call '*$R1(&t1024 .R3)' 

	# Prepare variables for the Callback function

	Push $R4
	Push $R3
	Push $R2
	Push $R1
	Push $R0

	StrCpy $R0 $R3

	# Call the Callback function

	Call $R4

	# Return variables

	Push $R5
	Exch
	Pop $R5

	Exch
	Pop $R0
	Exch
	Pop $R1
	Exch
	Pop $R2
	Exch
	Pop $R3
	Exch
	Pop $R4

	StrCmp $R5 "Stop" 0 +3
	Pop $R5
	Goto enumex

	Pop $R5

	enumnext: 

	# Next drive path

	IntOp $R1 $R1 + $R2 
	IntOp $R1 $R1 + 1 
	goto enumok 

	# End Search

	enumex:
	 
	# Free memory used for paths

	System::Free $R1 

	# Return original user variables

	Pop $0
	Pop $1
	Pop $R5
	Pop $R4
	Pop $R3
	Pop $R2
	Pop $R1
	Pop $R0

FunctionEnd ; DetectDrives

Function DriveSearchCallbackFunction
	IfFileExists "$R0$R6" FoundFile Continue
	FoundFile:
	  ;MessageBox "MB_OK" "Found it : $R0$R6"
	  StrCpy $R7 "$R0$R6"
	  Push "Stop"
	  goto End
	  
	Continue:
	  ;MessageBox "MB_OK" "Not found : $R0$R6"
	  Push "continue"
	End:
FunctionEnd ; DriveCallbackFunction



; Usage: Push on the filename to search for
; Return: Pop off a string, it will be "NOT FOUND" or a full path to the file
Function FindFileInRoot
	Exch $R6 ; Save old $R6 and get parameter
	Push $R7 ; Save old $R7
	Push "Hard Drives CDROM Drives RAM Drives Network Drives" ; Push desired drive types
	Push $0 ; Save old $0
	GetFunctionAddress $0 "DriveSearchCallbackFunction"
	Exch $0 ; Push callback function on stack and restore old $0
	StrCpy $R7 "NOT FOUND" ; Set default return value
	; At this point $R6 holds the file we're searching for, $R7 holds the return value
	Call DetectDrives
	; We now want to restore R6 and R7 and return R7
	Push $R7
	Pop $R6 ; $R6 now holds the return value
	Pop $R7 ; Restore saved $R7
	Exch $R6 ; Restore old $R6 and save return value
FunctionEnd ; FindFileInRoot

