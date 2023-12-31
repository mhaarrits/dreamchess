/*  DreamChess
 *  Copyright (C) 2006  The DreamChess project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_C99_VARARGS_MACROS
#define DBG_ERROR(...) do {dbg_error(__FILE__, __LINE__, __VA_ARGS__);} while (0)
#define DBG_WARN(...) do {dbg_warn(__FILE__, __LINE__, __VA_ARGS__);} while (0)
#define DBG_LOG(...) do {dbg_log(__FILE__, __LINE__, __VA_ARGS__);} while (0)
#elif defined(HAVE_GNUC_VARARGS_MACROS)
#define DBG_ERROR(args...) do {dbg_error(__FILE__, __LINE__, args);} while (0)
#define DBG_WARN(args...) do {dbg_warn(__FILE__, __LINE__, args);} while (0)
#define DBG_LOG(args...) do {dbg_log(__FILE__, __LINE__, args);} while (0)
#else
#define DBG_ERROR dbg_error
#define DBG_WARN dbg_warn
#define DBG_LOG dbg_log
#endif

#if defined(HAVE_C99_VARARGS_MACROS) || defined(HAVE_GNUC_VARARGS_MACROS)
#define HAVE_VARARGS_MACROS
#endif

void dbg_set_level(int level);
#ifdef HAVE_VARARGS_MACROS
void dbg_error(char *file, int line, const char *fmt, ...);
void dbg_warn(char *file, int line, const char *fmt, ...);
void dbg_log(char *file, int line, const char *fmt, ...);
#else
void dbg_error(const char *fmt, ...);
void dbg_warn(const char *fmt, ...);
void dbg_log(const char *fmt, ...);
#endif
