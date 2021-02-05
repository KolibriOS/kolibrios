static inline
int start_app(char *app_name, char *args){
#pragma pack(push, 1)
    struct file_op_t
    {
        uint32_t    fn;
        uint32_t    flags;
        char*       args;
        uint32_t    res1, res2;
        char        zero;
        char*       app_name  __attribute__((packed));
    } file_op;
    memset(&file_op, 0, sizeof(file_op));
    file_op.fn = 7;
    file_op.args = args;
    file_op.app_name = app_name;
    #pragma pack(pop)
    register int val;
    asm volatile ("int $0x40":"=a"(val):"a"(70), "b"(&file_op));

    return val;
}
