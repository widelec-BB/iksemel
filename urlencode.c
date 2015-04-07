/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen
** urlencode.c (URL encoding)
** Copyright (C) 2015 Filip 'widelec-BB' Maryjanski
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/
#include "common.h"
#include "iksemel.h"
#include "urlencode.h"

static const char *hex_table = "0123456789ABCDEF";

static inline int isUrlSafe(char c)
{
	return
	(
		(((c) >= 'a') && ((c) <= 'z')) ||
		(((c) >= 'A') && ((c) <= 'Z')) ||
		(((c) >= '0') && ((c) <= '9')) ||
		((c) == '.') ||
		((c) == '-') ||
		((c) == '_') ||
		((c) == '~')
	);
}

char *iks_urlencode_copy(const char *src, char *dst)
{
	if(!src || !dst)
		return NULL;

	while(*src)
	{
		if(isUrlSafe(*src))
			*dst = *src;
		else if(*src == ' ')
			*dst = '+';
		else
		{
			*dst++ = '%';
			*dst++ = hex_table[(*src >> 4) & 15];
			*dst = hex_table[*src & 15];
		}

		dst++;
		src++;
	}

	*dst = 0x00;

	return dst;
}

char *iks_urlencode_n_copy(const char *src, char *dst, int len)
{
	if(!src || !dst)
		return NULL;

	while(*src && len--)
	{
		if(isUrlSafe(*src))
			*dst = *src;
		else if(*src == ' ')
			*dst = '+';
		else
		{
			*dst++ = '%';
			*dst++ = hex_table[(*src >> 4) & 15];
			*dst = hex_table[*src & 15];
		}

		dst++;
		src++;
	}

	*dst = 0x00;

	return dst;
}

int iks_urlencode_len(const char *s)
{
	int res;

	if(!s)
		return 0;

	for(res = 0; *s; s++, res++)
		if(!isUrlSafe(*s))
			res += 2;

	return res;
}

char *iks_urlencode_new(const char *s)
{
	int len = iks_urlencode_len(s) + 1;
	char *res;

	if((res = iks_malloc(len)))
	{
		iks_urlencode_copy(s, res);
		res[len] = 0x00;
	}
	return res;
}

