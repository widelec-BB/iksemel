/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen
** morphos.c (implementations of stdlib functions 
** needed by iksemel for linking with -nostdlib on MorphOS)
** Copyright (C) 2014 Filip 'widelec-BB' Maryjanski
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include <clib/alib_protos.h>
#include <exec/rawfmt.h>
#include <proto/exec.h>
#include <proto/utility.h>
#include <sys/types.h>

extern struct Library *SysBase, *UtilityBase;

void *memset(void* ptr, int word, size_t size)
{
	size_t i;

	for(i = 0; i < size; i++)
		((unsigned char*)ptr)[i] = (unsigned char)word;

	return ptr;
}

char *strchr(const char *a, int c)
{
	char *s = (char*)a;
	while(*s && *s != (unsigned char)c)
		s++;

	if(*s == (unsigned char)c)
		return s;
	else
		return NULL;
}

size_t strlen(const char *s)
{
	char *v = (char*)s;

	while(*v)
		v++;
	return (size_t)(v - s);
}

void *memcpy(void *dst, const void *src, size_t len)
{
	CopyMem((APTR)src, (APTR)dst, (LONG)len);

	return dst;
}

void *malloc(size_t l)
{
	return AllocVec(l, MEMF_ANY);
}

void free(void *x)
{
	FreeVec(x);
}

int rand(void)
{
	return (RangeRand(65535) << 16) | RangeRand(65535);
}

int strncmp(const char *s, const char *d, size_t len)
{
	while(len--)
	{
		if(*s++ != *d++)
			return s[-1] - d[-1];
	}
	return NULL;
}

char *strstr(const char *s, const char *d)
{
	BYTE dch, sch;
	LONG len;

	dch = *d++;
	if(!dch)
		return (STRPTR) s;

	len = strlen(d);

	do
	{
		do
		{
			sch = *s++;
			if(!sch)
				return (char*) NULL;
		}
		while(sch != dch);
	}
	while(strncmp(s, d, len) != 0);

	return (char*)(s - 1);
}

int sprintf(char *dest, const char* fmt, ...)
{
	va_list args;

	va_start(args, fmt);

	VNewRawDoFmt((STRPTR)fmt, RAWFMTFUNC_STRING, dest, args);

	va_end(args);

	return strlen(dest);
}

int strcasecmp(const char *a, const char *b)
{
	return Stricmp((STRPTR)a, (STRPTR)b);
}

int strncasecmp(const char *a, const char *b, size_t l)
{
	return Strnicmp((STRPTR)a, (STRPTR)b, (LONG)l);
}

int strcmp(const char* s1, const char* s2)
{
	while(*s1 && (*s1 == *s2))
		s1++, s2++;
	return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char *strcopy(const char *s, char *d)
{
	while((*d++ = *s++));

	return (--d);
}


char *strdup(const char *str)
{
	char *n = NULL;

	if(str != NULL)
	{
		int len = strlen(str);

		if(len == 0) return NULL;

		if((n = malloc(len + sizeof(char))))
			strcopy(str, n);
	}

	return n;
}

