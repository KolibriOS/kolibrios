
extern void _pei386_runtime_relocator (void);

typedef void (*ctp)();
static void __do_global_ctors ()
{
  extern int __CTOR_LIST__;
  int *c = &__CTOR_LIST__;
  c++;
  while (*c)
    {
      ctp d = (ctp)*c;
      (d)();
      c++;
    }
}

int DllStartup(void *module, int reason)
{
    if(reason == 1)
    {
        _pei386_runtime_relocator();
        __do_global_ctors();
    };

    return 1;
};
