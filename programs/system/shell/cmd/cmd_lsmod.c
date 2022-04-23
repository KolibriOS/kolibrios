struct service_str{
		char  name[16];
		struct service_str* fd;
		struct service_str* bk;
		uint32_t base; // base addreass service for kernel_free 
		uint32_t entry; //unsigned drvEntry(int action, char *cmdline)
		uint32_t srv_proc; // static int __stdcall service_proc(ioctl_t *my_ctl)
		//void* srv_proc_kern;
	};

int cmd_lsmod(char param[]) {
    struct service_str* fd;
	struct service_str* bk;
	struct service_str srv_str;
	asm_inline(
        "int $0x40"
        :"=b"(fd), "=c"(bk)
        :"a"(68), "b"(31), "c"(1)
    );
	srv_str.fd = fd;
	printf("%-*s Hendler      Service_proc\n\r", 16, "  Name");
	while (1) {
		asm_inline(
        "int $0x40"
        ::"a"(68), "b"(31), "c"(2), "d"(fd), "D"(&srv_str)
		);
		printf("%-*s 0x%X   0x%X \n", 16, srv_str.name, fd, srv_str.srv_proc);
		if (fd == bk) break;
		fd = srv_str.fd;
	};
	
    return TRUE;
}
