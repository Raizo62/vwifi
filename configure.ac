AC_PREREQ([2.69])
AC_INIT([vwifi], [6.3], [https://github.com/Raizo62/vwifi/issues])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/vwifi-server.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Check for pkg-config
PKG_PROG_PKG_CONFIG

# Check for libnl using pkg-config
PKG_CHECK_MODULES([NETLINK], [libnl-3.0 >= 3.0 libnl-genl-3.0 >= 3.0])

# Check for pthread
AC_CHECK_LIB([pthread], [pthread_create], [], [
    AC_MSG_ERROR([pthread library not found])
])

# Basic header checks
AC_CHECK_HEADERS([stdio.h stdlib.h string.h])

# Output files
AC_CONFIG_FILES([
    Makefile
    src/Makefile
])

AC_OUTPUT 