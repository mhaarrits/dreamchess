SUBDIRS = gamegui mxml minizip menu
EXTRA_DIST = msgbuf.h
INCLUDES = -I$(top_srcdir)/src/include
AM_YFLAGS = -p san
noinst_LIBRARIES = @PIPE_LIB@ libsan.a
EXTRA_LIBRARIES = libpipe_unix.a libpipe_win32.a libpipe_beos.a \
	libpipe_mem.a
libpipe_beos_a_SOURCES = pipe_beos.c msgbuf.c
libpipe_unix_a_SOURCES = pipe_unix.c msgbuf.c
libpipe_win32_a_SOURCES = pipe_win32.c msgbuf.c
libpipe_mem_a_SOURCES = pipe_mem.c msgbuf.c
libsan_a_SOURCES = san_parse.y
