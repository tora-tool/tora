@echo off

set TORA_DIR=C:\Program Files\tora
set ORACLE_HOME=%TORA_DIR%\instantclient_11_1
set TNS_ADMIN=%TORA_DIR%

cd %TORA_DIR%\bin

start tora.exe


