
:: build Foo.dll

cl -c -DFOO_MAKE_DLL -Zm200 -O2 -DNDEBUG -MT -D_WINDLL  -EHsc -GR -W4 -wd4251 -I"." -I"..\..\include" Foo.cpp
link /NOLOGO /DLL /OUT:"Foo.dll" Foo.obj


:: build SingletonDll.dll

cl -c -DSINGLETONDLL_MAKE_DLL -DFOO_DLL -Zm200 -O2 -DNDEBUG -MT -D_WINDLL  -EHsc -GR -W4 -wd4251 -I"." -I"..\..\include" SingletonDll.cpp
link /NOLOGO /DLL /OUT:"SingletonDll.dll" SingletonDll.obj foo.lib


:: build Client.exe

cl -c -DSINGLETONDLL_DLL -DFOO_DLL -Zm200 -O2 -DNDEBUG -MT  -EHsc -GR -W4 -wd4251 -I"." -I"..\..\include" client.cpp
link /NOLOGO /SUBSYSTEM:CONSOLE /OUT:"Client-msvc.exe" Client.obj foo.lib SingletonDll.lib



del *.obj
del *.lib
del *.exp

