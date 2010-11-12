#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(qpdb, [0.1.1], [support@qpdb.com])
AC_CONFIG_SRCDIR([mime_types.cpp request_parser.hpp \
	mime_types.hpp server.cpp posi_main.cpp     \
	server.hpp file_hannpdler.cpp reply.cpp     \
	unyield.hpp file_handler.hpp reply.hpp      \
	yield.hpp header.hpp request.hpp            \
	request_parser.cpp main.cpp])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
AC_CHECK_LIB([boost], [Boost Library], [],
	AC_MSG_ERROR([Unable to find Boost C++ library]))

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([socket])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
