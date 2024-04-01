#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#define BUF_SIZE 256
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static int itoa(int n, char *s, unsigned int base) {
  assert(base <= 16);
  
  char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  unsigned unum;
  int i=0, j, k;
  if (base==10 && n < 0) {
    unum = (unsigned)(-n);
    s[i++] = '-';
  }
  else unum = (unsigned)n;
  do {
    s[i++] = index[unum%base];
    unum /= base;
  } while (unum);
  s[i] = '\0';
  if (s[0]=='-') k=1;
  else k = 0;
  char temp;
  for (j=k; j<=(i-k-1)/2; j++) {
    temp = s[j];
    s[j] = s[i-j-1];
    s[i-j-1] = temp;
  }
  return i;
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

int printf(const char *fmt, ...) {
  va_list ap;
  int ret;
  va_start(ap, fmt);

  char buf[BUF_SIZE];
  ret = vsprintf(buf, fmt, ap);
  va_end(ap);

  putstr(buf);
  
  return ret;
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
