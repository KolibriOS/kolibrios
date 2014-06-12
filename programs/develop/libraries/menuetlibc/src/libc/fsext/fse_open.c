/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <sys/fsext.h>

typedef struct FuncList {
  struct FuncList *next;
  __FSEXT_Function *function;
} FuncList;

static FuncList *func_list = 0;

int
__FSEXT_add_open_handler(__FSEXT_Function *_function)
{
  FuncList *new_func_list = (FuncList *)malloc(sizeof(FuncList));
  if (new_func_list == 0)
    return 1;
  new_func_list->next = func_list;
  func_list = new_func_list;
  func_list->function = _function;
  return 0;
}

int
__FSEXT_call_open_handlers(__FSEXT_Fnumber _function_number,
			   int *rv, /*va_list*/void* _args)
{
  FuncList *f;
  for (f=func_list; f; f=f->next)
    if (f->function(_function_number, rv, _args))
      return 1;
  return 0;
}
