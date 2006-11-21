/*
** return lower-case of c if upper-case, else c
*/
char tolower(char c)
{
  if(c<='Z' && c>='A') return (c+32);
  return (c);
}
