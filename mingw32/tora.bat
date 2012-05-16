@echo off

set TORA_DIR=C:\Program Files\tora-zd
set ORACLE_HOME=%TORA_DIR%\instantclient_11_2
set TNS_ADMIN=%TORA_DIR%
set PATH=%ORACLE_HOME%;%ORACLE_HOME%\bin;%PATH%

cd %TORA_DIR%\bin

start tora.exe

