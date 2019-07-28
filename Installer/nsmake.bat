@echo .
@echo .
@echo MAKING %1
@echo .
@echo .
if exist "c:\Program files\NSIS\MakeNSIS.exe" "c:\Program files\NSIS\MakeNSIS.exe" %1
if exist "c:\Program files (x86)\NSIS\MakeNSIS.exe" "c:\Program files (x86)\NSIS\MakeNSIS.exe" %1
if errorlevel 1 exit 1