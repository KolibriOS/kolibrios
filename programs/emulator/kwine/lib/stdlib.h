#ifndef _STDLIB_H
#define _STDLIB_H

int rand(void);
void srand(unsigned int seed);

void *malloc(size_t size);
int free(void *mem);
void* realloc(void *mem, size_t size);


#endif