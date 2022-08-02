#include <stddef.h>
#include <stdarg.h>

typedef struct {
  void  (*fct)(char character, void* arg);
  void* arg;
} out_fct_wrap_type;

typedef void (*out_fct_type)(char character, void* buffer, size_t idx, size_t maxlen);
 void _out_buffer(char character, void* buffer, size_t idx, size_t maxlen);
 void _out_null(char character, void* buffer, size_t idx, size_t maxlen);
 int _vsnprintf(out_fct_type out, char* buffer, const size_t maxlen, const char* format, va_list va);
