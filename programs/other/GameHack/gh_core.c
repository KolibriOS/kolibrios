int PID=-1;

int kdebugger_write(unsigned ID, unsigned n, unsigned addr, unsigned* buff)
{
    int num;
    __asm__ __volatile__(
        "int $0x40"
        :"=a"(num)
        :"a"(69), "b"(7), "c"(ID), "d"(n),"S"(addr),"D"(buff)
    );
    return num;
}

int kdebugger_read(unsigned ID, unsigned n, unsigned addr, unsigned* buff)
{
    int num;
    __asm__ __volatile__(
        "int $0x40"
        :"=a"(num)
        :"a"(69), "b"(6), "c"(ID), "d"(n),"S"(addr),"D"(buff)
    );
    return num;
}

void kdebugger_pause(unsigned ID)
{
    __asm__ __volatile__(
        "int $0x40"
        ::"a"(69), "b"(4), "c"(ID)
    );
}

void kdebugger_play(unsigned ID)
{
    __asm__ __volatile__(
        "int $0x40"
        ::"a"(69), "b"(5), "c"(ID)
    );
}

void kdebugger_disconnect(unsigned ID)
{
    __asm__ __volatile__(
        "int $0x40"
        ::"a"(69), "b"(3), "c"(ID)
    );
}

int load_game(char *app_name, char *args)
{    
    #pragma pack(push, 1)
    struct file_op_t
    {
        unsigned    fn;
        unsigned    flags;
        char*       args;
        unsigned    res1, res2;
        char        zero;
        char*       app_name  __attribute__((packed));
    } file_op;
    #pragma pack(pop)

    memset(&file_op, 0, sizeof(file_op));
    file_op.fn = 7;
    file_op.flags = 1;
    file_op.args = args;
    file_op.app_name = app_name;

    register int val;
    asm volatile ("int $0x40":"=a"(val):"a"(70), "b"(&file_op));
    return val;
}
