:: CopyExecutable.bat <Configuration> <TargetName> <TargetExt>

:: change to this batch file's directory
cd /d "%~dp0"

:: only try to copy if the target was an .exe
if "%~3"==".exe" (
	copy /B /Y "%~1\%~2.exe" .
	copy /B /Y "%~1\%~2.pdb" .
)

:: clear errorlevel (by running a null command), we don't care if this script fails
%COMSPEC% /c
