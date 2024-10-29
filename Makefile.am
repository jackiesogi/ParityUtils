# Include the common Makefile.am
ACLOCAL_AMFLAGS = -I m4

# Subdirectories to build
SUBDIRS = gnulib/lib

# Binary Program to build
bin_PROGRAMS = src/backup src/restore

# Source files to build
src_backup_SOURCES = src/backup.c \
					 src/parity.c \
                     lib/progname.c \
                     lib/propername.c 

# Automake compiler flags
AM_CPPFLAGS = -I$(srcdir)/src -I$(srcdir)/gnulib/lib
AM_CPPFLAGS += -I$(top_builddir)/lib -I$(top_srcdir)/lib

# LIBS = $(top_builddir)/lib/libgnu.a

# src_backup_LDADD = $(LIBS) $(LIBICONV) $(LIBUNISTRING) $(MBRTOWC_LIB)
LDADD = \
  -L./lib -lgnu \
  $(top_builddir)/lib/libgnu.a \
  $(HARD_LOCALE_LIB) \
  $(LIBPMULTITHREAD) \
  $(LIBPTHREAD) \
  $(LIBTHREAD) \
  $(LIBC32CONV) \
  $(LIBICONV) \
  $(LIBINTL) \
  $(LIBUNISTRING) \
  $(MBRTOWC_LIB) \
  $(SETLOCALE_LIB) \
  $(SETLOCALE_NULL_LIB) \
  @INTL_MACOSX_LIBS@

# src_restore_LDADD = $(LIBS)

# src_restore_SOURCES = src/restore.c \
#                       gnulib/lib/progname.c

EXTRA_DIST = m4/gnulib-cache.m4

dist-hook:
	cd gnulib && $(MAKE) dist-hook


