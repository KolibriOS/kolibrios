/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <search.h>

void
remque(struct qelem *e)
{
  if (!e)
    return;
  if (e->q_forw)
    e->q_forw->q_back = e->q_back;
  if (e->q_back)
    e->q_back->q_forw = e->q_forw;
}
