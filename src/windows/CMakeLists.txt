INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/windows/)

SET(WINDOWS_LIB_SOURCES
	dummy.cpp
	resource.h
	tora.ico
	tora.rc
)

SET(WINDOWS_LIB "windows_lib")
ADD_LIBRARY(${WINDOWS_LIB} STATIC ${WINDOWS_LIB_SOURCES})


# Error detected with generator Visual Studio 10 Win64
# > LINK : warning LNK4068: /MACHINE not specified; defaulting to X86
# > fatal error LNK1112: module machine type 'x64' conflicts with target machine type 'X86'
#
# There is no default value for static libraries and cmake isn't setting it either.
# We fix this by adding the flag manually.
if( CMAKE_SIZEOF_VOID_P EQUAL 8 AND MSVC )
	set_target_properties(${WINDOWS_LIB} PROPERTIES STATIC_LIBRARY_FLAGS "/machine:X64")
else()
	set_target_properties(${WINDOWS_LIB} PROPERTIES STATIC_LIBRARY_FLAGS "/machine:X86")
endif()

