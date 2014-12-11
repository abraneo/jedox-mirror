/* Generated from config.h.in.cmake by cmake.  */

/* Define if building universal (internal helper macro) */
#cmakedefine AC_APPLE_UNIVERSAL_BUILD

/* The normal alignment of `double', in bytes. */
#define ALIGNOF_DOUBLE @ALIGNOF_DOUBLE@

/* The normal alignment of `void*', in bytes. */
#define ALIGNOF_VOIDP @ALIGNOF_VOIDP@

/* Path separator character */
#define DIR_SEPARATOR_CHAR '@DIR_SEPARATOR_CHAR@'

/* Path separator string */
#define DIR_SEPARATOR_STR "@DIR_SEPARATOR_STR@"

/* enables https interface */
#cmakedefine ENABLE_HTTPS 1

/* enables https interface as shared library */
#cmakedefine ENABLE_HTTPS_MODULE 1

/* enables gpu server */
#cmakedefine ENABLE_GPU_SERVER 1

/* forces the use of select instead of poll */
#cmakedefine ENABLE_PIPE_COMM

/* forces the use of select instead of poll */
#cmakedefine ENABLE_SELECT

/* enables time profiling support */
#cmakedefine ENABLE_TIME_PROFILER

/* enables support for option --trace */
#cmakedefine ENABLE_TRACE_OPTION

/* Define to 1 if you have <boost/thread.hpp> */
#cmakedefine HAVE_BOOST_THREAD_HPP 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* have dlopen library function */
#cmakedefine HAVE_DLOPEN

/* Define to 1 if you have the `getrusage' function. */
#cmakedefine HAVE_GETRUSAGE 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the `boost_thread' library (-lboost_thread). */
#cmakedefine HAVE_LIBBOOST_THREAD 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to 1 if you have the <openssl/ssl.h> header file. */
#cmakedefine HAVE_OPENSSL_SSL_H 1

/* Define to 1 if you have the <poll.h> header file. */
#cmakedefine HAVE_POLL_H

/* Define if you have POSIX threads libraries and header files. */
#cmakedefine HAVE_PTHREAD

/* Define to 1 if you have the <regex.h> header file. */
#cmakedefine HAVE_REGEX_H 1

/* Define to 1 if you have the <signal.h> header file. */
#cmakedefine HAVE_SIGNAL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if the system has the type `suseconds_t'. */
#cmakedefine HAVE_SUSECONDS_T 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
#cmakedefine NO_MINUS_C_MINUS_O 1

/* overloading functions for size_t and uint32_t/uint64_t is possible */
#cmakedefine OVERLOAD_FUNCS_SIZE_T

/* overloading functions for size_t and long int is possible */
#cmakedefine OVERLOAD_FUNCS_SIZE_T_LONG

/* Name of package */
#define PACKAGE "@PACKAGE@"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "@PACKAGE_BUGREPORT@"

/* Define to the full name of this package. */
#define PACKAGE_NAME "@PACKAGE_NAME@"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "@PACKAGE_STRING@"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "@PACKAGE_TARNAME@"

/* Define to the version of this package. */
#define PACKAGE_VERSION "@PACKAGE_VERSION@"

/* Define to the home page for this package. */
#define PACKAGE_URL "@PACKAGE_URL@"

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
#cmakedefine PTHREAD_CREATE_JOINABLE

/* The size of `double', as computed by sizeof. */
#define SIZEOF_DOUBLE @SIZEOF_DOUBLE@

/* The size of `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T @SIZEOF_SIZE_T@

/* The size of `void*', as computed by sizeof. */
#define SIZEOF_VOIDP @SIZEOF_VOIDP@

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS 1

/* Version number of package */
#define VERSION "@PACKAGE_VERSION@"

/* Archtecture */
#define OUT_LIB_SUFFIX "@OUT_LIB_SUFFIX@"

/* TODO: check this options for cmake*/
/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
#cmakedefine WORDS_BIGENDIAN 1
# endif
#endif

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
#cmakedefine YYTEXT_POINTER 1

/* Define to `unsigned int' if <sys/types.h> does not define. */
#cmakedefine size_t @SIZE_T@

/* Define to 1 if you have the <udev.h> header file. */
#cmakedefine HAVE_LIBUDEV_H 1

/* Define to 1 if you have the libudev development library installed. */
#cmakedefine HAVE_LIBUDEV_LIB 1

/* Define to 1 if you want to enable google cpu profiler */
#cmakedefine ENABLE_GOOGLE_CPU_PROFILER

/* Enables mode for testing, e.g. Timer */
#cmakedefine ENABLE_TEST_MODE
