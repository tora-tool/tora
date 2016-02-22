@echo off
SET BUILD_ARCH=x64

REM SET DIR1=c:\Program Files (x86)\WiX Toolset v3.9\bin
REM SET DIR2=c:\Program Files\WiX Toolset v3.9\bin

REM IF EXIST "%DIR1%"\ SET PATH=%DIR1%;%PATH%
REM IF EXIST "%DIR2%"\ SET PATH=%DIR2%;%PATH%

set HKEY09="HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework\AssemblyFolders\WiX 3.9"
set HKEY10="HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework\AssemblyFolders\WiX 3.10"
set HKEY11="HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework\AssemblyFolders\WiX 3.11"

for /f "tokens=2*" %%a in ('REG QUERY %HKEY11% /ve') do set "AppPath=%%~b\..\bin\"
if exist "%AppPath%"\ SET PATH=%AppPath%;%PATH%

for /f "tokens=2*" %%a in ('REG QUERY %HKEY10% /ve') do set "AppPath=%%~b\..\bin\"
if exist "%AppPath%"\ SET PATH=%AppPath%;%PATH%

for /f "tokens=2*" %%a in ('REG QUERY %HKEY09% /ve') do set "AppPath=%%~b\..\bin\"
if exist "%AppPath%"\ SET PATH=%AppPath%;%PATH%

echo %AppPath%

set BUILD_NUMBER=

REM SVN version
REM for /F "tokens=1,2"  %%t  in ('svn info') do @if "%%t"=="Revision:" set BUILD_NUMBER=%%u
REM echo Build Number: %BUILD_NUMBER%

REM GIT version
for /F "tokens=1"  %%t  in ('git describe --long --tags --dirty --always') do set GIT_RELEASE=%%t
REM set GIT_RELEASE=v3.0alpha-30-g8e691f2-dirty
echo %GIT_RELEASE%
for /f "tokens=2 delims=- " %%G IN ("%GIT_RELEASE%") DO set BUILD_NUMBER=%%G
echo Build Number: %BUILD_NUMBER%

set BUILD_REL_PATH="%cd%\..\..\RelWithDebInfo\"
pushd %BUILD_REL_PATH%
set BUILD_ABS_PATH=%cd%
popd

echo %BUILD_ABS_PATH%

del heat.wxs
heat dir "../../RelWithDebInfo"  -var env.BUILD_ABS_PATH -cg ToraFiles -dr INSTALLLOCATION -suid -srd -sreg -gg -ag -out heat.wxs -t heat.xsl
candle.exe -arch x64 tora-qt5.wxs heat.wxs
light.exe -ext WixUIExtension -o tora3alpha.64bit.msi tora-qt5.wixobj heat.wixobj

@pause
