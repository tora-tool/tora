# This file was copied from CMake 2.6.
#
IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 2.5)
  FIND_PROGRAM(QT_LRELEASE_EXECUTABLE
    NAMES lrelease
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
    )

   MACRO(QT4_ADD_TRANSLATION _qm_files)
      FOREACH (_current_FILE ${ARGN})
         GET_FILENAME_COMPONENT(_abs_FILE ${_current_FILE} ABSOLUTE)
         GET_FILENAME_COMPONENT(qm ${_abs_FILE} NAME_WE)
         SET(qm "${CMAKE_CURRENT_BINARY_DIR}/${qm}.qm")

         ADD_CUSTOM_COMMAND(OUTPUT ${qm}
            COMMAND ${QT_LRELEASE_EXECUTABLE}
            ARGS ${_abs_FILE} -qm ${qm}
            DEPENDS ${_abs_FILE}
         )
         SET(${_qm_files} ${${_qm_files}} ${qm})
      ENDFOREACH (_current_FILE)
   ENDMACRO(QT4_ADD_TRANSLATION)
ENDIF(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 2.5)

