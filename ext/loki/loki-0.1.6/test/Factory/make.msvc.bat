
cl -c -Zm200 -O2 -DNDEBUG -MT -EHsc -GR -W4 -wd4710 -I"." -I"..\..\include" Factory.cpp

link /NOLOGO /SUBSYSTEM:CONSOLE /incremental:no /OUT:"main-msvc.exe" ..\..\lib\loki.lib Factory.obj

del *.obj

