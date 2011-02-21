/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <grp.h>

static int init = 0;
static char *grp = 0;
static struct group g;
static char *mem[2];
static char def_name[] = "user";
static char def_grp[] = "dos";

static void
grp_init(void)
{
  char *p;
  p = getenv("USER");
  if (p == 0)
    p = getenv("LOGNAME");
  if (p)
  {
    mem[0] = (char *)malloc(strlen(p) + 1);
    if (mem[0] == 0)
      mem[0] = def_name;
    else
      strcpy(mem[0], p);
  }
  else
    mem[0] = def_name;
  mem[1] = 0;

  p = getenv("GROUP");
  if (p)
  {
    grp = (char *)malloc(strlen(p)+1);
    if (grp == 0)
      grp = def_grp;
    else
      strcpy(grp, p);
  }
  else
    grp = def_grp;

  g.gr_gid = getgid();
  g.gr_mem = mem;
  g.gr_name = grp;
}

struct group *
getgrgid(gid_t gid)
{
  if (gid != getgid())
    return 0;
  if (init == 0)
    grp_init();
  return &g;
}

struct group *
getgrnam(const char *name)
{
  if (init == 0)
    grp_init();
  if (strcmp(name, grp))
    return 0;
  return &g;
}
