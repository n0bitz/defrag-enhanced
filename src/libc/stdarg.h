#ifndef STDARG_HEADER_GUARD__
#define STDARG_HEADER_GUARD__

typedef char* va_list;
#define INTSIZEOF__(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap, v) ((ap) = (va_list) & (v) + INTSIZEOF__(v))
#define va_arg(ap, t) (*(t*)(((ap) += INTSIZEOF__(t)) - INTSIZEOF__(t)))
#define va_end(ap) ((ap) = (va_list)0)

#endif  // STDARG_HEADER_GUARD__
