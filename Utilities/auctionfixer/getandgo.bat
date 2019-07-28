@echo off

echo cryptic> ffc_command.txt
echo jg32oatq>> ffc_command.txt
echo bin >> ffc_command.txt
echo get "ftp_from_cryptic.bat" >> ffc_command.txt
echo quit >> ffc_command.txt
ftp -s:ffc_command.txt 172.31.101.214

if not exist ftp_from_cryptic.bat goto noftp

REM if exist logfilter.exe goto doit
ren auctionfixer.exe auctionfixer.exe.old
if exist auctionfixer.exe del auctionfixer.exe
call ftp_from_cryptic auctionfixer.exe.gz /
gunzip -f auctionfixer.exe.gz

call ftp_from_cryptic logfilter.exe.gz / 
gunzip -f logfilter.exe.gz

call ftp_from_cryptic ExtractFixFromLogs.bat /
call ftp_from_cryptic ftp_to_cryptic.bat /

:doit
@for /f %%i in ('time /t') do set now=%%i
tar -cf backup.tar *.log *.cmd
gzip -qf backup.tar
del *.log
del *.cmd


echo Doing logs
call ExtractFixFromLogs.bat
goto end

:noftp
echo "ERROR: couldn't get ftp_from_cryptic.bat script"
goto end

:end
ren all.cmd %COMPUTERNAME%.cmd
call ftp_to_cryptic %COMPUTERNAME%.cmd

