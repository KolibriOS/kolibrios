
unsigned int strlen(const char *str);
char *strcpy(char *dest, const char *src);
void *memcpy(void *dest, const void *src, unsigned int n);
void *memset(void *s, char c, unsigned int n);
int strcmp(const char *str1, const char *str2);
char *strchr(const char *str, int ch);
char *strstr(const char *str1, const char *str2);

double floor(double x);
const char *DoubleToStr(double x, unsigned short digits = 5, bool crop_0 = false);
double StrToDouble(char *str);
long StrToInt(char *str);
