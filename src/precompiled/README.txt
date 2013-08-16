
Precompiled header files must NOT contain any #ifdef macros - at least on some platforms.
So we put a separate header file for each platform into this folder.
This will improve the compile time performace especially on Windows (as we also precompile windows.h)

See:
- tora3/CMakeList.txt
  Option USE_PCH (ON/OFF)

- tora3/src/CMakeList.txt 
  Variables: PCH_DEFINED, PCH_HEADER, PCH_SOURCE

- cmake/modules/PrecompiledHeader.cmake
  Note this usefull macro was copied from GitHub: https://gist.github.com/larsch/573926
  and modified slightly. 
    The suffix .cc is excluded from PCH usage.
    See files: parsing/*.cc (these files use function "emit()" which colidates with QT's emit macro).
    MACRO was turned into FUNCTION to make the variable FORCEINCLUDE "local".
  NOTE: this macro calls SET_SOURCE_FILES_PROPERTIES which set props for source within ALL tagets.
  There is NO way how to use PCH for specific target only.

PS: please keep QT headers sychronized.
