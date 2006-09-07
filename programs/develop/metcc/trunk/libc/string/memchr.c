void* memchr(const void* buf,int c,int count)
{
  int i;
  for (i=0;i<count;i++)
    if (*(char*)buf==c)
      return (void*)buf;
    else
      buf++;
  return (void*)0;
}
