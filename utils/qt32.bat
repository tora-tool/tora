@ECHO OFF
     
set DEV=D:\DEVEL
set QTDIR=%DEV%\qt-everywhere-opensource-src-4.8.5
set PATH=%SystemRoot%;%SystemRoot%\system32;d:\devel\TortoiseSVN\bin;D:\DEVEL\CMake2.8\bin;%QTDIR%\bin;D:\devel\doxygen\bin
     
echo Setting OpenSSL Env.
set OPENSSL=%DEV%\openssl
set PATH=%OPENSSL%\bin;%PATH%
set LIB=%OPENSSL%\lib
set INCLUDE=%OPENSSL%\include
     
rem echo Setting NASM Env.
rem set PATH=%DEV%\NASM;%PATH%
     
echo Setting DirectX Env.
set LIB=%DEV%\DXSDK2010\Lib\x86;%LIB%
set INCLUDE=%DEV%\DXSDK2010\Include;%INCLUDE%
     
echo Setting MSVC2010 Env.
set VSINSTALLDIR=C:\DEVEL\MSVCE2010
set VCINSTALLDIR=C:\DEVEL\MSVCE2010\VC
set DevEnvDir=%VSINSTALLDIR%\Common7\IDE
set PATH=%VCINSTALLDIR%\bin;%VSINSTALLDIR%\Common7\Tools;%VSINSTALLDIR%\Common7\IDE;%VCINSTALLDIR%\VCPackages;%PATH%
set INCLUDE=%VCINSTALLDIR%\include;%INCLUDE%
set LIB=%VCINSTALLDIR%\lib;%LIB%
set LIBPATH=%VCINSTALLDIR%\lib

echo Setting Windows SDK Env.
set WindowsSdkDir=%DEV%\XPSDK7.1
set PATH=%WindowsSdkDir%\Bin;%PATH%
set LIB=%WindowsSdkDir%\Lib;%LIB%
set INCLUDE=%WindowsSdkDir%\Include;%INCLUDE%
set TARGET_CPU=x86
     
echo Setting Framework Env.
set FrameworkVersion=v4.0.30319
set Framework35Version=v3.5
set FrameworkDir=%SystemRoot%\Microsoft.NET\Framework
set LIBPATH=%FrameworkDir%\%FrameworkVersion%;%FrameworkDir%\%Framework35Version%;%LIBPATH%
set PATH=%LIBPATH%;%PATH%;D:\Perl\bin
     
echo Env. ready.
     
title Qt Framework 4.8.5 Development Kit.
     
cd %DEV%
