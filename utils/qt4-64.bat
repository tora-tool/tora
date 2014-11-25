@ECHO OFF
 
set DEV=C:\Devel
set QTDIR=%DEV%\qt-everywhere-opensource-src-4.8.5.64bit
set PATH=%SystemRoot%;%SystemRoot%\system32;%QTDIR%\bin;c:\devel\CMake2.8\bin;C:\devel\Perl64\bin;c:\devel\TortoiseSVN\bin

echo Setting OpenSSL Env.
set OPENSSL=%DEV%\openssl-1.0.1e.64bit
set PATH=%OPENSSL%\bin;%PATH%
set LIB=%OPENSSL%\lib
set INCLUDE=%OPENSSL%\include
 
rem echo Setting NASM Env.
rem set PATH=%DEV%\NASM;%PATH%
 
echo Setting DirectX Env.
set LIB=%DEV%\DXSDK\Lib\x64;%LIB%
set INCLUDE=%DEV%\DXSDK\Include;%INCLUDE%
 
echo Setting Windows SDK Env.
set WindowsSdkDir=%DEV%\SDK7.1
set PATH=%WindowsSdkDir%\Bin;%PATH%
set LIB=%WindowsSdkDir%\Lib\x64;%LIB%
set INCLUDE=%WindowsSdkDir%\Include;%INCLUDE%
set TARGET_CPU=x64
 
echo Setting MSVC2010 Env.
set VSINSTALLDIR=%DEV%\MSVC10
set VCINSTALLDIR=%DEV%\MSVC10\VC
set DevEnvDir=%VSINSTALLDIR%\Common7\IDE
set PATH=%VCINSTALLDIR%\bin\amd64;%VSINSTALLDIR%\Common7\Tools;%VSINSTALLDIR%\Common7\IDE;%VCINSTALLDIR%\VCPackages;%PATH%
set INCLUDE=%VCINSTALLDIR%\include;%INCLUDE%
set LIB=%VCINSTALLDIR%\lib\amd64;%LIB%
set LIBPATH=%VCINSTALLDIR%\lib\amd64
 
echo Setting Framework Env.
set FrameworkVersion=v4.0.30319
set Framework35Version=v3.5
set FrameworkDir=%SystemRoot%\Microsoft.NET\Framework
set LIBPATH=%FrameworkDir%\%FrameworkVersion%;%FrameworkDir%\%Framework35Version%;%LIBPATH%
set PATH=%LIBPATH%;%PATH%
 
echo Env. ready.
 
title Qt Framework 4.8.5 Development Kit(64bit)
 
cd %DEV%
