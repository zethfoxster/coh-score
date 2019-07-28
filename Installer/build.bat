REM This is the batch file for building a version of the installer to be send to NCSoft in an automated fashion

SET VERSION=%~1
SET SOURCE_DIR=%~2\%VERSION%
SET UPDATER_EXE_DIR=%~3
SET UPDATER_EXE_NAME=%~4
SET CHECKSUM_FILE=%~5
SET COMPRESS_OPTION=%~6

REM Create appropriate Version.nsh file
attrib -r Version.nsh
echo !define VERSION "%VERSION%" > Version.nsh
echo !define SOURCE_DIR "%SOURCE_DIR%" >> Version.nsh
echo !define CHECKSUM_FILE "%CHECKSUM_FILE%" >> Version.nsh
REM    echo !define UPDATER_SOURCE_DIR "C:\game\tools\util" >> Version.nsh
echo !define UPDATER_SOURCE_DIR "%UPDATER_EXE_DIR%" >> Version.nsh
echo !define UPDATER_EXE_NAME "%UPDATER_EXE_NAME%" >> Version.nsh
if defined COMPRESS_OPTION echo !define %COMPRESS_OPTION% >> Version.nsh

REM Get latest on C:\game\src\installer
p4 sync C:\game\src\installer\...
p4 sync C:\game\code\coh\installer\...
TITLE Building installer...

REM Build the Installer!
md Release 2>nul
md Release\CD1 2>nul
md Release\CD2 2>nul
md Release\CD3 2>nul
md Release\CD4 2>nul
md Release\CD5 2>nul

for %%a in (coh.unwrapper.nsi coh.data1.nsi coh.data2.nsi coh.data3.nsi coh.data4.nsi coh.main.nsi coh.wrapper.nsi) do (
	"c:\Program files\NSIS\MakeNSIS.exe" %%a
	if errorlevel 1 goto error
)

@REM for %%a in (coh.data1.nsi coh.data2.nsi coh.data3.nsi coh.data4.nsi coh.data5.nsi) do (
@REM 	"c:\Program files\NSIS\MakeNSIS.exe" %%a
@REM 	if errorlevel 1 goto error
@REM )

@REM for %%a in ( coh.main.nsi ) do (
@REM 	"c:\Program files\NSIS\MakeNSIS.exe" %%a
@REM  	if errorlevel 1 goto error
@REM )

copy Release\CD1\FinishSetup.exe Release\CD2\FinishSetup.exe
if errorlevel 1 goto error
copy Release\CD1\FinishSetup.exe Release\CD3\FinishSetup.exe
if errorlevel 1 goto error
copy Release\CD1\FinishSetup.exe Release\CD4\FinishSetup.exe
if errorlevel 1 goto error
copy Release\CD1\FinishSetup.exe Release\CD5\FinishSetup.exe
if errorlevel 1 goto error

REM Removing old readme copying, making consistent
REM copy C:\game\src\installer\lang\README.RTF Release\CD1\
REM if errorlevel 1 goto error
REM copy C:\game\src\installer\lang\ReadmeUK.RTF Release\CD1\
REM if errorlevel 1 goto error
REM copy C:\game\src\installer\lang\ReadmeGerman.RTF Release\CD1\
REM if errorlevel 1 goto error
REM copy C:\game\src\installer\lang\ReadmeFrench.RTF Release\CD1\
REM if errorlevel 1 goto error

copy c:\game\src\Installer\README*.* Release\CD1\
copy c:\game\src\Installer\gr_lang\README-*.* Release\CD1\

@REM copy "N:\game\Install Images (nobackup)\_VAutorun\City_of_Heroes.exe" Release\CD1\
@REM if errorlevel 1 goto error
@REM copy "N:\game\Install Images (nobackup)\_VAutorun\CoH_Trailer.mpg" Release\CD1\
@REM if errorlevel 1 goto error

@REM md "Release\CD1\Game Manual" 2>nul
@REM copy "N:\employee resources\Marketing\CoH Manual\From IMGS For Review\Final Version\GameManual.pdf" 
@REM copy "N:\employee resources\Marketing\CoV\CoV Manual\CoV_Manual_lowres.pdf" "Release\CD1\Game Manual\Game Manual.pdf"
@REM if errorlevel 1 goto error

echo Installed build successfully!

goto end



:error
echo There was an error building the installer!
pause
exit /B 1
goto end

:end
