@echo off
SET BUILD_ARCH=x64

SET DIR1=c:\Program Files (x86)\WiX Toolset v3.9\bin
SET DIR2=c:\Program Files\WiX Toolset v3.9\bin

IF EXIST "%DIR1%"\ SET PATH=%DIR1%;%PATH%
IF EXIST "%DIR2%"\ SET PATH=%DIR2%;%PATH%

set BUILD_NUMBER=

REM for /F "tokens=1,2"  %%t  in ('svn info') do @if "%%t"=="Revision:" set BUILD_NUMBER=%%u
REM echo Build Number: %BUILD_NUMBER%


for /F "tokens=1"  %%t  in ('git describe --long --tags --dirty --always') do set GIT_RELEASE=%%t
REM set GIT_RELEASE=v3.0alpha-30-g8e691f2-dirty
echo %GIT_RELEASE%

for /f "tokens=2 delims=- " %%G IN ("%GIT_RELEASE%") DO set BUILD_NUMBER=%%G
echo Build Number: %BUILD_NUMBER%

candle.exe tora-qt5.wxs
light.exe -ext WixUIExtension -o tora3alpha.64bit.msi tora-qt5.wixobj

@pause

