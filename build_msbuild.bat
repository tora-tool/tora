
msbuild.exe TOra.sln /t:Clean /p:Configuration=RelWithDebInfo
msbuild.exe TOra.sln /t:ZERO_CHECK /maxcpucount:6 /p:Configuration=RelWithDebInfo
msbuild.exe TOra.sln /t:revisiontag /maxcpucount:6 /p:Configuration=RelWithDebInfo
msbuild.exe TOra.sln /t:tora;poracle /maxcpucount:6 /p:Configuration=RelWithDebInfo

rem msbuild.exe TOra.sln /t:Clean /p:Configuration=Release
rem msbuild.exe TOra.sln /t:ZERO_CHECK /maxcpucount:6 /p:Configuration=Release
rem msbuild.exe TOra.sln /t:revisiontag /maxcpucount:6 /p:Configuration=Release
rem msbuild.exe TOra.sln /t:tora;poracle /maxcpucount:6 /p:Configuration=Release

msbuild.exe INSTALL.vcxproj /p:Configuration=RelWithDebInfo
