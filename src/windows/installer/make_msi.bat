@echo off
SET BUILD_ARCH=x86

SET DIR1=c:\Program Files (x86)\WiX Toolset v3.9\bin
SET DIR2=c:\Program Files\WiX Toolset v3.9\bin

IF EXIST "%DIR1%"\ SET PATH=%DIR1%;%PATH%
IF EXIST "%DIR2%"\ SET PATH=%DIR2%;%PATH%

set BUILD_NUMBER=

for /F "tokens=1,2"  %%t  in ('svn info') do @if "%%t"=="Revision:" set BUILD_NUMBER=%%u

echo Build Number: %BUILD_NUMBER%

candle.exe tora.wxs
light.exe -ext WixUIExtension -o tora3alpha.32bit.msi tora.wixobj

@pause
