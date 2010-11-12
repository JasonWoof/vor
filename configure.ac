#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(VoR, 0.5.5, jason@jasonwoof.com, vor)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
PKG_CHECK_MODULES([SDL], [sdl >= 1.2])
PKG_CHECK_MODULES([SDL_IMAGE], [SDL_image >= 1.2],,
	[SDL_IMAGE_LIBS="-lSDL_image"]) # <-- else
PKG_CHECK_MODULES([SDL_MIXER], [SDL_mixer >= 1.2],,
	[SDL_MIXER_LIBS="-lSDL_mixer"]) # <-- else
# Make sure SDL_image was compiled with png support
AC_CHECK_LIB([SDL_image], [IMG_LoadPNG_RW])
# Make sure SDL_mixer was compiled with ogg vorbis support
AC_CHECK_LIB([SDL_mixer], [Mix_InitOgg])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h stddef.h stdlib.h string.h unistd.h time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit memset sqrt strdup nanosleep])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
