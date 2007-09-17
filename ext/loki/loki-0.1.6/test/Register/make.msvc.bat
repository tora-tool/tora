
:: build Foo.dll

cl -c -Zm200 -O2 -DNDEBUG -MT  -EHsc -GR -W4 -wd4251 -I"." -I"..\..\include" Foo.cpp
link /lib /NOLOGO /OUT:"Foo.lib" Foo.obj


:: build main.exe

cl -c -Zm200 -O2 -DNDEBUG -MT -D_WINDLL  -EHsc -GR -W4 -wd4251 -I"." -I"..\..\include" main.cpp
link /NOLOGO /SUBSYSTEM:CONSOLE /OUT:"main-msvc.exe" main.obj foo.lib  ..\..\lib\loki.lib



del *.obj
del *.lib


