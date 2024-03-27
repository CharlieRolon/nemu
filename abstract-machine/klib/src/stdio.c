#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
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

  for ( ; *format != '\0'; ++format) {
    if (*format != '%') {
      *str = *format;
      ++str;
    }
    else {
      switch(*(++format)) {
        case '%': *str = *format; ++str; break;
        case 'd': str += itoa(va_arg(pArgs, int), str, 10); break;
        case 's': 
          char *s = va_arg(pArgs, char*);
          strcpy(str, s);
          str += strlen(str);
          break;
      }
    }
  }
  *str = '\0';
  va_end(pArgs);

  return str - start;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
