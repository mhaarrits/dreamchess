#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.56)
AC_INIT([dreamchess],[0.1.0-goat])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_LEX
AC_CHECK_PROGS(YACC, 'bison -y', [${am_missing_run} bison -y])

AC_C_INLINE

if test "x$cross_compiling" = xyes; then
	test "${CC_FOR_BUILD+set}" = set || CC_FOR_BUILD="$CC"
else
	CC_FOR_BUILD="$CC"
fi
AC_SUBST(CC_FOR_BUILD)

# Check for varargs macro support (borrowed from glib)
AC_MSG_CHECKING(for ISO C99 varargs macros)
AC_TRY_COMPILE([],[
	int a(int p1, int p2, int p3);
	#define call_a(...) a(1, __VA_ARGS__)
	call_a(2, 3);
	], have_c99_varargs=yes, have_c99_varargs=no)
AC_MSG_RESULT($have_c99_varargs)

AC_MSG_CHECKING(for GNU C varargs macros)
AC_TRY_COMPILE([],[
	int a(int p1, int p2, int p3);
	#define call_a(params...) a(1, params)
	call_a(2, 3);
	], have_gnuc_varargs=yes, have_gnuc_varargs=no)
AC_MSG_RESULT($have_gnuc_varargs)

if test x"$have_c99_varargs" = xyes; then
	AC_DEFINE([HAVE_C99_VARARGS_MACROS], [1], [Define to 1 if the compiler supports C99 varargs macros.])
fi

if test x"$have_gnuc_varargs" = xyes; then
	AC_DEFINE([HAVE_GNUC_VARARGS_MACROS], [1], [Define to 1 if the compiler supports GNU C varargs macros.])
fi

# Checks for libraries.
dnl Check for SDL
AC_ARG_WITH([sdl], [  --without-sdl           do not use SDL])
AC_ARG_WITH([gl], [  --without-gl            do not use GL])
AC_ARG_ENABLE([linked-engine], [  --enable-linked-engine  statically link with `dreamer' engine], [enable_linked_engine=yes])

AC_DEFUN([AC_CHECK_SDL_HEADER],
	OLD_CFLAGS="${CFLAGS}"
	CFLAGS="${CFLAGS} ${SDL_CFLAGS}"
	AC_MSG_CHECKING([for $1])
	AC_COMPILE_IFELSE(
		[AC_LANG_SOURCE([[#include "$1"]])],
		[AC_MSG_RESULT([yes]) ; $2],
		[AC_MSG_RESULT([no]) ; $3]
		)
	CFLAGS="${OLD_CFLAGS}"
	)

AC_DEFUN([AC_NEEDS],
	OLD_LIBS="${LIBS}"
	LIBS="${LIBS} -l$1 $6"
	[AC_CACHE_CHECK([whether lib$1 needs -l$2], ac_$1_needs_$2,
		AC_TRY_LINK_FUNC($3,
			[ac_$1_needs_$2="no" ; $5],
			[ac_$1_needs_$2="yes" ; $4]
			)
		)]
	LIBS="${OLD_LIBS}"
	)

AM_PATH_SDL([1.2.0], [have_sdl="yes"], [have_sdl="no"])
SDL_OLD_LIBS="${SDL_LIBS}"

AC_CHECK_SDL_HEADER([SDL_image.h], [have_sdl_image_h="yes"],
	[have_sdl_image_h="no"]
	)

AC_CHECK_SDL_HEADER([SDL_opengl.h],
	[have_sdl_opengl_h="yes"], [have_sdl_opengl_h="no"]
	)

AC_CHECK_LIB(m, sqrt, [have_m="yes" M_LIBS="-lm"], [have_m="no"])

AC_CHECK_LIB(z, compress, [have_z="yes" Z_LIBS="-lz"],
	[have_z="no"]
	)

AC_CHECK_LIB(png, png_create_read_struct, [have_png="yes"
	PNG_LIBS="-lpng"], [have_png="no"], [${Z_LIBS} ${M_LIBS}]
	)

if test x"$have_png" = xyes -a x"$have_m" = xyes; then
	AC_NEEDS(png, m, png_create_read_struct,
		[PNG_LIBS="${PNG_LIBS} -lm"], [], [${Z_LIBS}]
		)
fi

if test x"$have_png" = xyes -a x"$have_z" = xyes; then
	AC_NEEDS(png, z, png_create_read_struct,
		[PNG_LIBS="${PNG_LIBS} -lz"], [],
		[${M_LIBS}]
		)
fi

AC_CHECK_LIB(jpeg, jpeg_CreateCompress,
	[have_jpeg="yes" JPEG_LIBS="-ljpeg"], [have_jpeg="no"], []
	)

AC_CHECK_LIB([SDL_image], [IMG_LoadPNG_RW],
	[have_sdl_image="yes" SDL_IMAGE_LIBS="-lSDL_image"],
	[have_sdl_image="no"],
	[${PNG_LIBS} ${JPEG_LIBS} ${SDL_LIBS}]
	)

if test x"$have_sdl_image" = xyes -a x"$have_png" = xyes; then
	AC_NEEDS(SDL_image, png, IMG_LoadPNG_RW,
		[SDL_IMAGE_LIBS="${SDL_IMAGE_LIBS} ${PNG_LIBS}"],
		[], [${JPEG_LIBS} ${SDL_LIBS}]
		)
fi

if test x"$have_sdl_image" = xyes -a x"$have_jpeg" = xyes; then
	AC_NEEDS(SDL_image, jpeg, IMG_LoadPNG_RW,
		[SDL_IMAGE_LIBS="${SDL_IMAGE_LIBS} ${JPEG_LIBS}"],
		[], [${PNG_LIBS} ${SDL_LIBS}]
		)
fi

have_gl="no"

if test "$host_os" = "mingw32"; then
	AC_CHECK_LIB(shlwapi, main, [],
		[AC_MSG_ERROR([Cannot find shlwapi library.])])
	AC_CHECK_LIB(opengl32, main,
		AC_CHECK_LIB(glu32, main,
			[OPENGL_LIBS="-lopengl32 -lglu32" have_gl="yes"]
			)
		)
elif test x"$CC" = xkos-cc; then
	AC_CHECK_LIB(gl, glBegin,
		[OPENGL_LIBS="-lgl" have_gl="yes"]
		)
else
	AC_CHECK_LIB(GL, glBegin,
		AC_CHECK_LIB(GLU, gluUnProject,
			[OPENGL_LIBS="-lGL -lGLU" have_gl="yes"],
			AC_CHECK_LIB(GL, gluUnProject,
				[OPENGL_LIBS="-lGL" have_gl="yes"]
				)
			)
		)
fi

if test x"$have_sdl_image_h" = xyes -a \
	x"$have_sdl_opengl_h" = xyes -a \
	x"$have_sdl" = xyes -a \
	x"$have_sdl_image" = xyes -a \
	x"$have_gl" = xyes; then supported_ui_sdlgl="yes"
else supported_ui_sdlgl="no"
fi

if test x"$supported_ui_sdlgl" = xno; then
	with_ui_sdlgl="no"
elif test x"$with_gl" = xno -o x"$with_sdl" = xno; then
	with_ui_sdlgl="no"
else
	with_ui_sdlgl="yes"
fi

if test x"$have_sdl_image_h" = xyes -a \
	x"$have_sdl" = xyes -a \
	x"$have_sdl_image" = xyes; then supported_ui_sdl="yes"
else supported_ui_sdl="no"
fi

if test x"$supported_ui_sdl" = xno; then
	with_ui_sdl="no"
elif test x"$with_sdl" = xno; then
	with_ui_sdl="no"
else
	with_ui_sdl="no"
fi

AC_DEFUN([AC_PRINT_LIB],
	if test x"$2" = xyes; then
		echo "$1|  yes ($3)"
	else
		echo "$1|  no"
	fi
	)

if test x"$with_ui_sdlgl" = xyes; then
	SDL_LIBS="${SDL_IMAGE_LIBS} ${SDL_TTF_LIBS} ${SDL_LIBS} ${OPENGL_LIBS}"
	AC_DEFINE([WITH_UI_SDLGL], [1], [Define to 1 if the sdlgl user interface driver should be compiled.])
elif test x"$with_ui_sdl" = xyes; then
	SDL_LIBS="${SDL_IMAGE_LIBS} ${SDL_TTF_LIBS} ${SDL_LIBS}"
elif test x"$enable_linked_engine" != xyes; then
	SDL_CFLAGS=""
	SDL_LIBS=""
fi

if test x"$with_ui_sdl" = xyes; then
	AC_DEFINE([WITH_UI_SDL], [1], [Define to 1 if the sdl user interface driver should be compiled.])
fi

if test x"$enable_linked_engine" = xyes -a x"$have_sdl" = xyes -a x"$with_sdl" != xno; then
	DREAMCHESS_LIBS="libs/libpipe_mem.a dreamer/libdreamer.a"
	AC_SUBST(DREAMER_SDL_CFLAGS, $SDL_CFLAGS)
	AC_SUBST(PIPE_LIB, "libpipe_mem.a")
	AC_DEFINE([COMM_SDL_THREADS], [1], [Define to 1 if you want UI<->engine communication via SDL threads.])
elif test x"$enable_linked_engine" = xyes; then
	AC_MSG_ERROR([Option `--enable-linked-engine' requires SDL, which was not found or disabled.])
else
	AC_SUBST(DREAMER, "dreamer\$(EXEEXT)")
	if test "$host_os" = "mingw32"; then
		AC_SUBST(PIPE_LIB, "libpipe_win32.a")
		AC_CHECK_TOOL(WINDRES, windres)
		if test -n "$WINDRES"; then
			AC_SUBST(WINDRES)
			AC_SUBST(WIN32_RC, "win32_rc.\$(OBJEXT)")
		fi
		DREAMCHESS_LIBS="libs/libpipe_win32.a"
		DREAMER_LIBS="../libs/libpipe_win32.a"
		AC_DEFINE([COMM_PIPE_WIN32], [1], [Define to 1 if you want UI<->engine communication via Win32 pipes.])
	elif test "$host_os" = "beos"; then
		AC_SUBST(PIPE_LIB, "libpipe_beos.a")
		AC_CHECK_TOOL(RC, rc)
		AC_CHECK_TOOL(XRES, xres)
		if test -n "$RC" -a -n "$XRES"; then
			AC_SUBST(RC)
			AC_SUBST(XRES)
			beos_rc="yes"
		fi
		DREAMCHESS_LIBS="libs/libpipe_beos.a"
		DREAMER_LIBS="../libs/libpipe_beos.a"
		AC_DEFINE([COMM_PIPE_UNIX], [1])
	else
		AC_SUBST(PIPE_LIB, "libpipe_unix.a")
		DREAMCHESS_LIBS="libs/libpipe_unix.a"
		DREAMER_LIBS="../libs/libpipe_unix.a"
		AC_DEFINE([COMM_PIPE_UNIX], [1], [Define to 1 if you want UI<->engine communication via UNIX pipes.])
	fi
fi

if test x"$CC" = xkos-cc; then
	DREAMCHESS_LIBS="${DREAMCHESS_LIBS} libs/minizip/libminizip.a libs/menu/libmenu.a"
	MINIZIP_LIB="libminizip.a"
	MENU_LIB="libmenu.a"
	AC_SUBST(MINIZIP_CFLAGS, "-DZ_BUFSIZE=2048 -DUNZ_BUFSIZE=2048")
fi

AM_CONDITIONAL(BEOS_RC, test x"$beos_rc" = x"yes")
AC_SUBST(DREAMCHESS_LIBS)
AC_SUBST(DREAMER_LIBS)
AC_SUBST(MINIZIP_LIB)
AC_SUBST(MENU_LIB)

if test "$host_os" != "mingw32"; then
	AC_SUBST(DATADIR, "-DDATADIR=\\\"\$(pkgdatadir)\\\"")
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h getopt.h])
AC_CHECK_FUNCS(getopt_long strdup vsnprintf usleep)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_VOLATILE

# Needed for Mini-XML.
AH_TOP(/* Needed for Mini-XML. */
[#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>])

AH_BOTTOM(/* Needed for Mini-XML. */
[#ifndef HAVE_STRDUP
extern char *mxml_strdup(const char *);
#define strdup mxml_strdup
#endif

extern char *mxml_strdupf(const char *, va_list);

#ifndef HAVE_VSNPRINTF
extern int mxml_vsnprintf(char *, size_t, const char *, va_list);
#define vsnprintf mxml_vsnprintf
#endif])

# Checks for library functions.

AC_CONFIG_FILES([Makefile
		desktop/Makefile
		doc/Makefile
		man/Makefile
		m4/Makefile
		pkg/Makefile
		pkg/dc-goat/Makefile
		pkg/win32/Makefile
		src/Makefile
		src/libs/Makefile
		src/libs/gamegui/Makefile
		src/libs/mxml/Makefile
		src/libs/minizip/Makefile
		src/libs/menu/Makefile
		src/include/Makefile
		src/include/gamegui/Makefile
		src/include/minizip/Makefile
		src/ui_sdlgl/Makefile
		src/dreamer/Makefile])
AC_OUTPUT

cat <<_ACEOF

User interface driver requirements:

sdlgl
-----

  Requirement  | Found
------------------------
"SDL_image.h"  |  $have_sdl_image_h
"SDL_opengl.h" |  $have_sdl_opengl_h
_ACEOF
AC_PRINT_LIB([libSDL         ], [$have_sdl], [$SDL_OLD_LIBS])
AC_PRINT_LIB([libSDL_image   ], [$have_sdl_image], [$SDL_IMAGE_LIBS])
AC_PRINT_LIB([libGL          ], [$have_gl], [$OPENGL_LIBS])

cat <<_ACEOF

Supported:        $supported_ui_sdlgl
Build:            $with_ui_sdlgl

You can now run \`make'.

_ACEOF
