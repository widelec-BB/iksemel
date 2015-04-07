#ifndef __MORPHOS_H__
#define __MORPHOS_H__

#include <errno.h>

#define USE_DEFAULT_IO 1

void *memset(void* ptr, int word, size_t size);
char *strchr(const char *a, int c);
size_t strlen(const char *s);
void *memcpy(void *dst, const void *src, size_t len);
void *malloc(size_t l);
void free(void *x);
int rand(void);
int strncmp(const char *s, const char *d, size_t len);
char *strstr(const char *s, const char *d);
int sprintf(char *dest, const char* fmt, ...);
int strcasecmp(const char *a, const char *b);
int strncasecmp(const char *a, const char *b, size_t l);
int strcmp(const char* s1, const char* s2);
char *strcopy(const char *s, char *d);
char *strdup(const char *str);

#endif /* __MORPHOS_H__ */