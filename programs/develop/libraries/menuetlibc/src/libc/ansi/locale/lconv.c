/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <locale.h>
#include <limits.h>

static char ESTR[] = "";
static char DSTR[] = ".";

static struct lconv __lconv_ = {
  ESTR,
  DSTR,
  ESTR,
  ESTR,
  ESTR,
  ESTR,
  ESTR,
  ESTR,
  ESTR,
  ESTR,
  CHAR_MAX,
  CHAR_MAX,
  CHAR_MAX,
  CHAR_MAX,
  CHAR_MAX,
  CHAR_MAX,
  CHAR_MAX,
  CHAR_MAX
};

struct lconv *localeconv()
{
  return &__lconv_;
}
