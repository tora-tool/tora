/* antlr3config.h.in.cmake Generated from antlr3config.h.in and adopted for cmake */

/* Define if ANTLR debugger not required */
#cmakedefine ANTLR3_NODEBUGGER

/* Define if 64 bit mode required */
#cmakedefine ANTLR3_USE_64BIT ${ANTLR3_USE_64BIT}

/* Name of package */
/*#cmakedefine PACKAGE "${PACKAGE}"*/

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT "${PACKAGE_BUGREPORT}"

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME "${PACKAGE_NAME}"

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING "${PACKAGE_STRING}"

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME "${PACKAGE_TARNAME}"

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION "${PACKAGE_VERSION}"

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS ${STDC_HEADERS}

/* Version number of package */
#cmakedefine VERSION "${VERSION}"

/* Define to 1 if you have the <arpa/nameser.h> header file. */
#cmakedefine HAVE_ARPA_NAMESER_H ${HAVE_ARPA_NAMESER_H}

/* Define to 1 if you have the <ctype.h> header file. */
#cmakedefine HAVE_CTYPE_H ${HAVE_CTYPE_H}

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H ${HAVE_DLFCN_H}

/* Define to 1 if the system has the type `intptr_t'. */
#cmakedefine HAVE_INTPTR_T ${HAVE_INTPTR_T}

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H ${HAVE_INTTYPES_H}

/* Define to 1 if you have the <malloc.h> header file. */
#cmakedefine HAVE_MALLOC_H ${HAVE_MALLOC_H}

/* Define to 1 if you have the `memmove' function. */
#cmakedefine HAVE_MEMMOVE ${HAVE_MEMMOVE}

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H ${HAVE_MEMORY_H}

/* Define to 1 if you have the `memset' function. */
#cmakedefine HAVE_MEMSET ${HAVE_MEMSET}

/* Define to 1 if you have the <netdb.h> header file. */
#cmakedefine HAVE_NETDB_H ${HAVE_NETDB_H}

/* Define to 1 if you have the <netinet/in.h> header file. */
#cmakedefine HAVE_NETINET_IN_H ${HAVE_NETINET_IN_H}

/* Define to 1 if you have the <netinet/tcp.h> header file. */
#cmakedefine HAVE_NETINET_TCP_H ${HAVE_NETINET_TCP_H}

/* Define to 1 if you have the <resolv.h> header file. */
#cmakedefine HAVE_RESOLV_H ${HAVE_RESOLV_H}

/* Define to 1 if you have the <socket.h> header file. */
#cmakedefine HAVE_SOCKET_H ${HAVE_SOCKET_H}

/* Define to 1 if you have the <stdarg.h> header file. */
#cmakedefine HAVE_STDARG_H ${HAVE_STDARG_H}

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H ${HAVE_STDINT_H}

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H ${HAVE_STDLIB_H}

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H ${HAVE_STRINGS_H}

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H ${HAVE_STRING_H}

/* Define to 1 if you have the <sys/malloc.h> header file. */
#cmakedefine HAVE_SYS_MALLOC_H ${HAVE_SYS_MALLOC_H}

/* Define to 1 if you have the <sys/socket.h> header file. */
#cmakedefine HAVE_SYS_SOCKET_H ${HAVE_SYS_SOCKET_H}

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H ${HAVE_SYS_STAT_H}

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H ${HAVE_SYS_TYPES_H}

/* Define to 1 if the system has the type `uintptr_t'. */
#cmakedefine HAVE_UINTPTR_T ${HAVE_UINTPTR_T}

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H ${HAVE_UNISTD_H}

/* Define to 1 if you have the `accept' function. */
#cmakedefine HAVE_ACCEPT ${HAVE_ACCEPT}

/* Define to 1 if you have the `strdup' function. */
#cmakedefine HAVE_STRDUP ${HAVE_STRDUP}


/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
#cmakedefine _UINT32_T

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
#cmakedefine _UINT64_T

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
#cmakedefine _UINT8_T

/* Define to empty if `const' does not conform to ANSI C. */
#cmakedefine const

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#cmakedefine inline
#endif

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine int16_t

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine int32_t

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine int64_t

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
#cmakedefine int8_t

/* Define to the type of a signed integer type wide enough to hold a pointer,
   if such a type exists, and if the system does not define it. */
#cmakedefine intptr_t

/* Define to `unsigned int' if <sys/types.h> does not define. */
#cmakedefine size_t

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine uint16_t

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine uint32_t

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine uint64_t

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine uint8_t

/* Define to the type of an unsigned integer type wide enough to hold a
   pointer, if such a type exists, and if the system does not define it. */
#cmakedefine uintptr_t
