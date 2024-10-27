# Automake 的 ACLOCAL 標誌，指定 m4 文件的目錄
ACLOCAL_AMFLAGS = -I m4

# 遞歸構建子目錄 (gnulib/lib)
SUBDIRS = gnulib/lib

# 指定要編譯的可執行文件
bin_PROGRAMS = src/backup src/restore

# 指定 backup 和 restore 的源文件
src_backup_SOURCES = src/backup.c \
					 src/parity.c \
                     gnulib/lib/progname.c

src_restore_SOURCES = src/restore.c \
                      gnulib/lib/progname.c

# Automake 編譯選項，指定 include 頭文件的目錄
AM_CPPFLAGS = -I$(srcdir)/src -I$(srcdir)/gnulib/lib

# 包含 gnulib 的文件 (由 gnulib-tool 自動生成)
EXTRA_DIST = m4/gnulib-cache.m4

# 指定 gnulib 的構建步驟
dist-hook:
	cd gnulib && $(MAKE) dist-hook

