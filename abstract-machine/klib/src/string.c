#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t n;
  for (n=0; s[n] != '\0'; n++)
    if (n > MAX_STRING_LEN - 1) return -1;
  return n;
}

char *strcpy(char *dst, const char *src) {
  assert(strlen(src) != -1);
  size_t i;
  for (i = 0; src[i] != '\0'; i++) {
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  assert(strlen(src) != -1);
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

static void reverse(char *s, int len) {
  char *end = s + len - 1;
  char tmp;
  while (s < end) {
    tmp = *s;
    *s = *end;
    *end = tmp;
  }
}

static int itoa(int n, char *s, int base) {
  assert(base <= 16);
  
  int i = 0, sign = n, bit;
  if (sign < 0) n = -n;
  do {
    bit = n % base;
    if (bit >= 10) s[i++] = 'a' + bit - 10;
    else s[i++] = '0' + bit;
  } while ((n/=base) > 0);
  if (sign < 0) s[i++] = '-';
  s[i] = '\0';
  reverse(s, i);

  return i;
}

int sprintf(char *str, const char *format, ...) {
  va_list pArgs;
  va_start(pArgs, format);
  char *start = str;

  for ( ; *format != '\0'; format++) {
    if (*format != '%') {
      *start = *format;
      str++;
    }
    else {
      switch(*(++format)) {
        case '%': *str = *format; str++; break;
        case 'd': str += itoa(va_arg(pArgs, int), str, 10); break;
        case 's': 
          char *s = va_arg(pArgs, char*);
          strcpy(str, s);
          str += strlen(s);
          break;
      }
    }
  }
  *str = '\0';
  va_end(pArgs);

  return str - start;
}

#endif