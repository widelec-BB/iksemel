/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdio.h>

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#elif HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifndef errno
extern int errno;
#endif

#ifdef __DEBUG__
#define LIBNAME "iksemel"
#include <clib/debug_protos.h>
#define tprintf(template, ...) KPrintF((CONST_STRPTR)LIBNAME" " __FILE__ " %d: " template, __LINE__ , ##__VA_ARGS__)
#define ENTER(...) KPrintF((CONST_STRPTR)LIBNAME " enters: %s\n", __PRETTY_FUNCTION__)
#define LEAVE(...) KPrintF((CONST_STRPTR)LIBNAME " leaves: %s\n", __PRETTY_FUNCTION__)
#else
#define tprintf(...)
#define ENTER(...)
#define LEAVE(...)
#endif /* __DEBUG__ */

#include "finetune.h"
