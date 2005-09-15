@echo off

echo ==============================
echo =   Configuring QScintilla   =
echo ==============================

rem ----------------- WARNING ----------------- 
rem -  These variables supposed to be set by  - 
rem -  by user according to their environment - 
rem -------------------------------------------

set QSCINTILLA=D:\_Projects_\qscintilla
set MINGW=C:\Development\msys\mingw
set QTDIR=D:\_Projects_\qt-3\mingw

rem ------------ END OF WARNING ------------

set QMAKESPEC=win32-g++
set MAKE=mingw32-make

set PATH=%MINGW%\bin;%QTDIR%\bin;%PATH%

echo Generating Makefiles ...

rem generating makefiles from qmake files
cd %QSCINTILLA%\designer
qmake "CONFIG+=thread" designer 
cd %QSCINTILLA%\qt
qmake "CONFIG+=thread rtti" qscintilla.pro

echo Making QScintilla DLL

rem making qscintilla dll
cd %QSCINTILLA%\qt
%MAKE%

rem copying files to QT
copy qextscintilla*.h %QTDIR%\include
copy qscintilla*.qm %QTDIR%\translations
copy %QTDIR%\lib\qscintilla.dll %QTDIR%\bin 

echo Making Designer plugin

rem making plugin
cd %QSCINTILLA%\designer
%MAKE%

cd %QSCINTILLA%

echo Done!
