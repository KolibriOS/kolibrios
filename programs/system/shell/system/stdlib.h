
#define RAND_MAX 0x7FFFU

#define isspace(c) ((c)==' ')
#define abs(i) (((i)<0)?(-(i)):(i))

#define random(num) ((rand()*(num))/((RAND_MAX+1)))

void* malloc(unsigned size);
void  free(void* pointer);
void* realloc(void* pointer, unsigned size);

void srand (unsigned seed);
int rand (void);
