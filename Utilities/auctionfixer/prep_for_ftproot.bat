
for %%I in (auctionfixer.pl) do (
pushd n:\bin\bin
call pp.bat -o %%~dpIauctionfixer.exe %%~dpIauctionfixer.pl
popd )
gzip -f auctionfixer.exe

for %%i in ("auctionfixer.exe.gz" "ExtractFixFromLogs.bat") DO xcopy /y %%i \\cohftp\ftproot\cryptic

:done