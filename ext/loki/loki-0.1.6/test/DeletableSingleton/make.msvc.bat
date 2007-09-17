
::  DeletableSingleton.cpp

cl -c -Zm200 -O2 -DNDEBUG -MT -EHsc -GR -W4 -wd4710 -I"." -I"..\..\include" DeletableSingleton.cpp

link /NOLOGO /SUBSYSTEM:CONSOLE /incremental:no /OUT:"DeletableSingleton-msvc.exe" ..\..\lib\loki.lib DeletableSingleton.obj 


del *.obj

