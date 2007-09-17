if not exist tmp\ mkdir tmp

cl -c -Zm200 -O2 -DNDEBUG -MT -EHsc -GR -W4 -wd4996 -wd4710 -I"." -I"..\..\include"  main.cpp

link /NOLOGO /SUBSYSTEM:CONSOLE /incremental:no /OUT:"main-msvc.exe"  main.obj

del *.obj

