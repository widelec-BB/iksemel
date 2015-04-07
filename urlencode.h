#ifndef __URLENCODE_H__
#define __URLENCODE_H__

char *iks_urlencode_copy(const char *src, char *dst);
char *iks_urlencode_n_copy(const char *src, char *dst, int len);
int iks_urlencode_len(const char *s);
char *iks_urlencode_new(const char *s);

#endif /* __URLENCODE_H__ */
