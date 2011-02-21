/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <search.h>

void
insque(struct qelem *e, struct qelem *p)
{
  if (!e || !p)
    return;
  e->q_back = p;
  e->q_forw = p->q_forw;
  p->q_forw->q_back = e;
  p->q_forw = e;
}
