
/*
 * skip.c
 *
ÿ */
 
#define MEMSIZE	65536
#define MASK8	0xFF

unsigned char mem[MEMSIZE/8];

void mem_reset(void)
{
  int i;

  for (i=0; i < sizeof(mem); i++)
    mem[i] = MASK8;
}

void mem_skip(int min, int max)
{
  int pc;

  for (pc = min; pc < max; pc++)
    mem[pc >> 3] &= ~(1 << (pc & 7));
}

void print_mem(void)
{
  int fd = creat("opmap", 0666);
  int i;

#if 0
  for (i=0; i < sizeof(mem); i++)
    if (mem[i] != MASK8) printf("%04x: %02x\n", i * 8, mem[i]);
#else
  write(fd, mem, sizeof(mem));
#endif

  close(fd);
}

void get_range(char *s, int *min, int *max)
{
  sscanf(s, "%x-%x", min, max);
  printf("%x-%x\n", *min, *max);
}

main(int argc, char **argv)
{
  int i, min, max;

  mem_reset();
  
  for (i=1; i < argc; i++) {
    get_range(argv[i], &min, &max);
    mem_skip(min, max);
  }

  print_mem();
}
