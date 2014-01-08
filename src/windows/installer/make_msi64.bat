@echo off
SET BUILD_ARCH=x64

SET DIR1=c:\Program Files (x86)\WiX Toolset v3.7\bin
SET DIR2=c:\Program Files\WiX Toolset v3.7\bin

IF EXIST "%DIR1%"\ SET PATH=%DIR1%;%PATH%
IF EXIST "%DIR2%"\ SET PATH=%DIR2%;%PATH%

candle.exe tora.wxs
light.exe -ext WixUIExtension -o tora3alpha.64bit.msi tora.wixobj

@pause

