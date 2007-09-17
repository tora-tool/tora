@ECHO OFF

echo -
echo - Use make.msvc.bat dll to link with dynamic runtile library
echo -

if "%1"=="dll" (
	set MTMD=MD
	set OUT_EXE=main-dll-msvc.exe
	echo - using dynamic library: /MD
	echo -
) else (
	set MTMD=MT
	set OUT_EXE=main-msvc.exe
	echo - using static library: /MT
	echo -
)

@ECHO ON



:: SmallObjBench.cpp

cl -c -DNDEBUG -Zm200 -O2 -%MTMD% -EHsc -GR -W4 -wd4710 -wd4100 -I"." -I"..\..\include" -I"..\..\include\loki" SmallObjBench.cpp 
:: ..\..\src\SmallObj.cpp ..\..\src\Singleton.cpp

link /NOLOGO /SUBSYSTEM:CONSOLE /incremental:no /OUT:%OUT_EXE% SmallObjBench.obj ..\..\lib\loki.lib
::SmallObj.obj tmp\Singleton.obj



:: SmallSingleton.cpp

cl -c -DNDEBUG -Zm200 -O2 -%MTMD% -EHsc -GR -W4 -wd4710 -I"." -I"..\..\include" -I"..\..\include\loki"  SmallObjSingleton.cpp

link /NOLOGO /SUBSYSTEM:CONSOLE /incremental:no /OUT:SmallObjSingleton-msvc.exe ..\..\lib\loki.lib SmallObjSingleton.obj 


del *.obj


@ECHO OFF
set MTMD=
set OUT_EXE=

