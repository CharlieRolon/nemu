#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  assert(s != NULL);
  size_t i = 0;
  while (s[i] != '\0') {
    ++i;
  }
  return i;
}

char *strcpy(char *dst, const char *src) {
  assert(src != NULL && dst != NULL);
  char *res = dst;
  do {
    *dst = *src;
    dst++;
    src++;
  } while (*src != '\0');
  return res;
}

char *strncpy(char *dst, const char *src, size_t n) {
  assert(src != NULL && dst != NULL);
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i++) {
    dst[i] = src[i];
  }
  for ( ; i < n; i++) {
    dst[i] = '\0';
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t dst_len = strlen(dst);
  size_t i;

  for (i = 0; src[i] != '\0'; i++)
    dst[dst_len + i] = src[i];
  dst[dst_len + i] = '\0';
  
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  assert(s1 && s2);
  int ret = 0;
  for ( ; !(ret = *(unsigned char *)s1 - *(unsigned char *)s2) && *s2; s1++, s2++);
  if (ret < 0) ret = -1;
  else if (ret > 0) ret = 1;
  return ret;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  assert(s1 && s2);
  int ret = 0;
  size_t i;
  for (i = 0; i < n && !(ret = *(unsigned char *)s1 - *(unsigned char *)s2) && *s2; i++, s1++, s2++);
  if (ret < 0) ret = -1;
  else if (ret > 0) ret = 1;
  return ret;
}

void *memset(void *s, int c, size_t n) {
  assert(s);
  size_t i;
  char *s_func = (char *)s;
  for (i = 0; i < n; i++)
    s_func[i] = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  assert(dst && src);
  size_t i;
  char *pdst = (char *)dst;
  char *psrc = (char *)src;
  if (pdst < psrc) {
    for (i = 0; i < n; i++)
      pdst[i] = psrc[i];
  }
  else {
    for (i = n-1; i >= 0; i--)
      pdst[i] = psrc[i];
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  assert(out && in);
  size_t i;
  char *pout = (char *)out;
  char *pin  = (char *)in ;
  for (i = 0; i < n; i++)
    pout[i] = pin[i];
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  assert(s1 && s2);
  int ret = 0;
  size_t i;
  for (i = 0; i < n && !(ret = *(unsigned char *)s1 - *(unsigned char *)s2); i++, s1++, s2++);
  if (ret < 0) ret = -1;
  else if (ret > 0) ret = 1;
  return ret;
}

#endif