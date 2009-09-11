/*
	some libC function working with memory
*/

static void *memmove(void *dst,const void *src,size_t length);
static size_t strlen(const char *s);
static char* strchr(const char *string, int c);
static char* strrchr(const char *string, int c);
static char* strstr(const char *s1,const char *s2);
static int strcmp(const char*,const char*);
static int strncmp(const char* string1, const char* string2, int count);

static int vsnprintf(char *dest, size_t size,const char *format,va_list ap);
static int cdecl snprintf(char *dest, size_t size, const char *format,...);
static int cdecl sprintf(char *dest,const char *format,...);

