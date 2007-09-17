
::  main.cpp

cl -c -Zm200 -O2 -DNDEBUG -MT -EHsc -GR -W4 -wd4710 -I"." -I"..\..\include"  main.cpp

link /NOLOGO /SUBSYSTEM:CONSOLE /incremental:no /OUT:"main-msvc.exe" ..\..\lib\loki.lib main.obj



::  main2.cpp

cl -c -Zm200 -O2 -DNDEBUG -MT -EHsc -GR -W4 -wd4710 -I"." -I"..\..\include"  main2.cpp

link /NOLOGO /SUBSYSTEM:CONSOLE /incremental:no /OUT:"main2-msvc.exe" ..\..\lib\loki.lib main2.obj


del *.obj

