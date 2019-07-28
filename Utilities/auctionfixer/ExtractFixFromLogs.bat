REM recovery occurred for files from 5/8 8PM CDT to 5/9 9AM CDT
REM The log files are in the time zone of their servers
mkdir c:\auctionfix
cd c:\auctionfix
del *.cmd


REM all of 07-31-2007
REM all of 08-01-2007
for %%i in ("d:\city of heroes\logs\logserver\db_2007-07-31-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-01-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd

REM 08-02 to 12:31:18
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-00-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-01-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-02-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-03-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-04-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-05-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-06-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-07-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-08-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-09-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-10-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-11-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd
for %%i in ("d:\city of heroes\logs\logserver\db_2007-08-02-12-*.log.gz") do auctionfixer.exe "%%i" c:\auctionfix\%%~ni.cmd

copy *.cmd all.cmd