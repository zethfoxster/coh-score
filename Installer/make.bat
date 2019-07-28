md Release 2>nul
md Release\CD1 2>nul
md Release\CD2 2>nul
md Release\CD3 2>nul
md Release\CD4 2>nul
md Release\CD5 2>nul
@REM for %%a in (coh.unwrapper.nsi coh.data1.nsi coh.data2.nsi coh.data3.nsi coh.data4.nsi coh.data5.nsi coh.main.nsi coh.wrapper.nsi) do call nsmake %%a
@REM for %%a in (coh.main.nsi coh.wrapper.nsi) do call nsmake %%a
for %%a in (coh.main.nsi) do call nsmake %%a
@REM for %%a in (coh.wrapper.nsi) do call nsmake %%a
@REM for %%a in (coh.data1.nsi coh.data2.nsi) do call nsmake %%a
@REM for %%a in (coh.data1.nsi) do call nsmake %%a
@REM for %%a in (bgimageexample.nsi) do call nsmake %%a
copy Release\CD1\FinishSetup.exe Release\CD2\FinishSetup.exe
copy Release\CD1\FinishSetup.exe Release\CD3\FinishSetup.exe
copy Release\CD1\FinishSetup.exe Release\CD4\FinishSetup.exe
copy Release\CD1\FinishSetup.exe Release\CD5\FinishSetup.exe
copy C:\game\src\installer\README.RTF Release\CD1\
