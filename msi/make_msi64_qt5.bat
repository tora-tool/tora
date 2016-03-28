@echo off

REM
REM Try to detect various versions of Wix
REM
set HKEY11="HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework\AssemblyFolders\WiX 3.11"
for /f "tokens=2*" %%a in ('REG QUERY %HKEY11% /ve') do set "AppPath=%%~b\..\bin\"
if exist "%AppPath%"\ SET PATH=%AppPath%;%PATH%

set HKEY10="HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework\AssemblyFolders\WiX 3.10"
for /f "tokens=2*" %%a in ('REG QUERY %HKEY10% /ve') do set "AppPath=%%~b\..\bin\"
if exist "%AppPath%"\ SET PATH=%AppPath%;%PATH%

set HKEY09="HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework\AssemblyFolders\WiX 3.9"
for /f "tokens=2*" %%a in ('REG QUERY %HKEY09% /ve') do set "AppPath=%%~b\..\bin\"
if exist "%AppPath%"\ SET PATH=%AppPath%;%PATH%
echo %AppPath%

set BUILD_NUMBER=

REM SVN version (not used anymore)
REM for /F "tokens=1,2"  %%t  in ('svn info') do @if "%%t"=="Revision:" set BUILD_NUMBER=%%u
REM echo Build Number: %BUILD_NUMBER%

REM GIT version (not used anymore)
REM for /F "tokens=1"  %%t  in ('git describe --long --tags --dirty --always') do set GIT_RELEASE=%%t
REM echo %GIT_RELEASE%
REM for /f "tokens=2 delims=- " %%G IN ("%GIT_RELEASE%") DO set BUILD_NUMBER=%%G
REM echo Build Number: %BUILD_NUMBER%

set BUILD_REL_PATH="%cd%\..\TARGET\Tora"
pushd %BUILD_REL_PATH%
set BUILD_ABS_PATH=%cd%
popd

echo %BUILD_ABS_PATH%

if exist "OSD Ivan Brezina.p12" (
set /P PASS="Sign password:"
)

if exist "OSD Ivan Brezina.p12" (
signtool sign /v /f "OSD Ivan Brezina.p12" /P %PASS% ^
 /d "TOra is an open source SQL IDE for Oracle, MySQL and PostgreSQL" ^
 /du "https://github.com/tora-tool/tora/wiki" ^
 /t http://timestamp.verisign.com/scripts/timstamp.dll ^
 %BUILD_ABS_PATH%\*.exe %BUILD_ABS_PATH%\*.dll
)

del heat.wxs
heat dir %BUILD_ABS_PATH% -var env.BUILD_ABS_PATH -cg ToraFiles -dr APPLICATIONFOLDER -suid -srd -sreg -gg -ag -out heat.wxs -t heat.xsl
candle.exe -arch x64 tora-qt5.wxs heat.wxs MyWixUI_Advanced.wxs
light.exe -ext WixUIExtension -o Tora3.beta.64bit.msi tora-qt5.wixobj heat.wixobj MyWixUI_Advanced.wixobj

if exist "OSD Ivan Brezina.p12" (
signtool sign /v /f "OSD Ivan Brezina.p12" /P %PASS% ^
 /d "TOra is an open source SQL IDE for Oracle, MySQL and PostgreSQL" ^
 /du "https://github.com/tora-tool/tora/wiki" ^
 /t http://timestamp.verisign.com/scripts/timstamp.dll ^
 *.msi
)

@pause

pushd %cd%
cd %BUILD_ABS_PATH%
cd ..
zip -r Tora3.beta.64bit.zip Tora
popd
