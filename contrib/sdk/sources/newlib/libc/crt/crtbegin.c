
extern void _pei386_runtime_relocator (void);

int DllStartup(void *module, int reason)
{
    _pei386_runtime_relocator();
    return 1;
};

void  ___dll_start ()

{
    _pei386_runtime_relocator();

}
