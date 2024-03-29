#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#define BUF_SIZE 4096
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static void reverse(char *s, int len) {
  char *end = s + len - 1;
  char tmp;
  while (s < end) {
    tmp = *s;
    *s = *end;
    *end = tmp;
  }
}

static int itoa(int n, char *s, unsigned int base) {
  assert(base <= 16);
  
  int i = 0, sign = n, bit;
  if (sign < 0) n = -n;
  do {
    bit = n % base;
    if (bit >= 10) s[i++] = 'a' + bit - 10;
    else s[i++] = '0' + bit;
  } while ((n = n / base) > 0);
  if (sign < 0) s[i++] = '-';
  s[i] = '\0';
  reverse(s, i);

  return i;
}

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *start = out;
  memset(out, '\0', BUF_SIZE);

  for ( ; *fmt != '\0'; ++fmt) {
    if (*fmt != '%') {
      *out = *fmt;
      ++out;
    }
    else {
      switch(*(++fmt)) {
        case '%': *out = *fmt; ++out; break;
        case 'd': out += itoa(va_arg(ap, int), out, 10); break;
        case 's': 
          char *s = va_arg(ap, char*);
          strcpy(out, s);
          out += strlen(out);
          break;
      }
    }
  }
  *out = '\0';

  return out - start;
}

int sprintf(char *str, const char *format, ...) {
  va_list ap;
  int ret;

  va_start(ap, format);
  ret = vsprintf(str, format, ap);
  va_end(ap);

  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
