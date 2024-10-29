AC_PREREQ([2.69])              # require Autoconf 2.69 or later

AC_CONFIG_SRCDIR([lib/Makefile.am])
AC_CONFIG_HEADERS([lib/config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AC_INIT([parityutils], [1.0])   # initialize Autoconf
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_PROG_CC                     # detect C compiler
gl_EARLY

AC_CONFIG_FILES([
                 Makefile
                 lib/Makefile
])   # Generate Makefile

gl_INIT
AC_OUTPUT                     # Output the configuration file
