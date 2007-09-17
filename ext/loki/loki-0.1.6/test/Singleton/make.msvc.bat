
::  Phoenix.cpp

cl -c -Zm200 -O2 -DNDEBUG -MT -EHsc -GR -W4 -wd4710 -I"." -I"..\..\include" Phoenix.cpp

link /NOLOGO /SUBSYSTEM:CONSOLE /incremental:no /OUT:"Phoenix-msvc.exe" ..\..\lib\loki.lib Phoenix.obj 




::  Dependencies.cpp

cl -c -Zm200 -O2 -DNDEBUG -MT -EHsc -GR -W4 -wd4710 -I"." -I"..\..\include" Dependencies.cpp

link /NOLOGO /SUBSYSTEM:CONSOLE /incremental:no /OUT:"Dependencies-msvc.exe" ..\..\lib\loki.lib Dependencies.obj


del *.obj

