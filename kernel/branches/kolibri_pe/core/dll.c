
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>
#include <slab.h>
#include <pe.h>

int __stdcall strncmp(const char *s1, const char *s2, size_t n);

extern int __stdcall mnt_exec(void *raw, size_t raw_size, char *path,
              char *cmdline, u32_t flags) asm ("mnt_exec");

static dll_t core_dll;

static char* strupr(char *str )
{
    char *p;
    unsigned char c;

    p = str;
    while( (c = *p) )
    {
        if( c >= 'a' && c <= 'z' )
            *p = c - 'a' + 'A';
        ++p;
    }

    return( str );
}

void * memcpy(void * _dest, const void *_src, size_t _n)
{
int d0, d1, d2;
 __asm__ __volatile__(
	"rep ; movsl\n\t"
	"testb $2,%b4\n\t"
	"je 1f\n\t"
	"movsw\n"
	"1:\ttestb $1,%b4\n\t"
	"je 2f\n\t"
	"movsb\n"
	"2:"
	: "=&c" (d0), "=&D" (d1), "=&S" (d2)
	:"0" (_n/4), "q" (_n),"1" ((long)_dest),"2" ((long)_src)
	: "memory");
 return (_dest);
}

size_t strlen(const char *str)
{
int d0;
register int __res;
__asm__ __volatile__(
	"repne\n\t"
	"scasb\n\t"
	"notl %0\n\t"
	"decl %0"
	:"=c" (__res), "=&D" (d0) :"1" (str),"a" (0), "0" (0xffffffff));
return __res;
}

void init_core_dll()
{
    PIMAGE_DOS_HEADER        dos;
    PIMAGE_NT_HEADERS32      nt;
    PIMAGE_EXPORT_DIRECTORY  exp;

    dos =  (PIMAGE_DOS_HEADER)LOAD_BASE;
    nt  =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);
    exp =  MakePtr(PIMAGE_EXPORT_DIRECTORY,LOAD_BASE,
                   nt->OptionalHeader.DataDirectory[0].VirtualAddress);

    list_initialize(&core_dll.link);

    core_dll.img_base = LOAD_BASE;
    core_dll.img_size = nt->OptionalHeader.SizeOfImage;
    core_dll.img_md   = NULL;

    core_dll.img_hdr  = nt;
    core_dll.img_sec  = MakePtr(PIMAGE_SECTION_HEADER,nt, sizeof(IMAGE_NT_HEADERS32));
    core_dll.img_exp  = MakePtr(PIMAGE_EXPORT_DIRECTORY,LOAD_BASE,
                        nt->OptionalHeader.DataDirectory[0].VirtualAddress);
    core_dll.img_name = strupr(MakePtr(char*, LOAD_BASE, exp->Name));

    DBG("%s base %x size %x sections %d exports %x\n",
        core_dll.img_name, core_dll.img_base,
        core_dll.img_size, nt->FileHeader.NumberOfSections,
        core_dll.img_exp );
};


dll_t * find_dll(const char *name)
{
    dll_t* dll = &core_dll;

    do
    {
        if( !strncmp(name,dll->img_name,16))
            return dll;

        dll = (dll_t*)dll->link.next;

    }while(&dll->link !=  &core_dll.link);

    return NULL;
};


typedef struct
{
  char         srv_name[16];  //        ASCIIZ string
  u32_t        magic;         // +0x10  'SRV '
  size_t       size;          // +0x14  size of structure SRV
  void        *fd;            // +0x18  next SRV descriptor
  void        *bk;            // +0x1C  prev SRV descriptor
  addr_t       base;          // +0x20  service base address
  addr_t       entry;         // +0x24  service START function
  void        *srv_proc;      // +0x28  main service handler
}srv_t;

typedef srv_t* __stdcall  drv_entry_t(int);

srv_t* __fastcall load_pe_driver(const char *path)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;

    drv_entry_t   *drv_entry;
    md_t          *md;
    srv_t         *srv;

    md = load_image(path);

    if( ! md )
        return 0;

    if( link_image( md->base ) )
    {
        dos = (PIMAGE_DOS_HEADER)md->base;
        nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

        drv_entry = MakePtr(drv_entry_t*, md->base,
                            nt->OptionalHeader.AddressOfEntryPoint);

        srv = drv_entry(1);

        if(srv != NULL)
            srv->entry = nt->OptionalHeader.AddressOfEntryPoint + md->base;

        return srv;
    }
    else
    {
        md_free( md );
        return NULL;
    }
}

typedef struct
{
    int a_type;
    union
    {
        long  a_val;
        void *a_ptr;
        void  (*a_fcn)( ) ;
    }a_un;
}auxv_t;

#define AUX_COUNT       0

typedef struct
{
    int     argc;       /*  always 2                                */
    char   *path;       /*  argv[0]   program path                  */
    char   *cmdline;    /*  argv[1]   command  line. May be null    */
    u32_t   sep1;       /*  separator.  must be zero                */
    char   *env;        /*  single environment string               */
    u32_t   sep2;       /*  separator.  must be zero                */
    auxv_t  aux[1];     /*  aux. AT_NULL for now                    */
}exec_stack_t;


addr_t new_app_space(void);

int __stdcall pe_app_param(char *path, void *raw, addr_t ex_pg_dir,
                          addr_t ex_stack_page) asm ("pe_app_param");

int sys_exec(char *path, char *cmdline, u32_t flags)
{
    addr_t        ex_pg_dir;
    addr_t        ex_stack_tab;
    addr_t        ex_stack_page;
    addr_t        ex_pl0_stack;

    exec_stack_t *ex_stack;
    int           stack_size;
    char         *ex_path;
    char         *ex_cmdline = NULL;

    size_t        raw_size;
    u32_t        *raw;

    int pathsize = 0;
    int cmdsize  = 0;
    int envsize  = 0;

    u32_t tmp;

    DBG("\nexec %s cmd %s flags %x\n", path, cmdline, flags);

    if( ! path)
    {
        DBG("invalid path\n");
        return;
    };

    raw = load_file(path, &raw_size);

    if( ! raw )
        return -5;                                      /* FIXME */

    if( (raw[0] == 0x554E454D) &&
        ( ( raw[1] == 0x31305445) ||
          ( raw[1] == 0x30305445) ) )

    {
        DBG("leagacy Kolibri application\n");
        int tmp =  mnt_exec(raw, raw_size, path, cmdline, flags);
        return tmp;
    }

    if( ! validate_pe(raw, raw_size) )
    {
        DBG("invalid executable file %s\n", path);
        mem_free(raw);
        return -31;
    }

    pathsize = strlen(path)+1;

    if( cmdline )
        cmdsize = strlen(cmdline)+1;

    stack_size = sizeof(exec_stack_t) + pathsize +
                 cmdsize + envsize + AUX_COUNT*sizeof(auxv_t);

    stack_size = (stack_size + 15) & ~15;               /* keep stack aligned */

    DBG("stacksize %d\n", stack_size);

    if( stack_size > 4096 )
    {
        DBG("command line too long\n");
        return -30;
    }

    ex_pg_dir      = new_app_space();

    if( !ex_pg_dir )
    {
        mem_free(raw);
        return -30;                                    /* FIXME          */
    };

    ex_stack_tab   = ex_pg_dir + 4096;
    ex_pl0_stack   = ex_pg_dir + 4096 * 2;

    ex_stack_page  = core_alloc(0);                    /* 2^0 = 1 page   */

    if( ! ex_stack_page )
    {
        core_free(ex_stack_tab);
        mem_free(raw);
        return -30;                                    /* FIXME          */
    };

    __asm__ __volatile__ (
    "xorl %%eax, %%eax      \n\t"
    "rep stosl"
    :"=c"(tmp),"=D"(tmp)
    :"c"(1024),"D"(ex_stack_page + OS_BASE)
    :"eax","cc");

    ((u32_t*)(ex_stack_tab+OS_BASE))[1023] = ex_stack_page | 7;

    ex_stack = (exec_stack_t*)(ex_stack_page + OS_BASE
                               + PAGE_SIZE - stack_size);
    ex_stack->argc = 2;

    ex_path = MakePtr(char*, ex_stack, sizeof(exec_stack_t)+AUX_COUNT*sizeof(auxv_t));

    memcpy(ex_path, path, pathsize);
    ex_stack->path = (char*)(((addr_t)ex_path & 0xFFF) + 0x7FCFF000);  /* top of stack */

    if( cmdline )
    {
        ex_cmdline = ex_path + pathsize;
        memcpy(ex_cmdline, cmdline, cmdsize);
        ex_stack->cmdline = ex_stack->path + pathsize;
    };

/*
    ex_stack.env = null
    ex_stack.aux[0] = AT_NULL
 */

    DBG("create stack at %x\n\tpath %x\n\tcmdline %x\n",
         ex_stack, ex_stack->path, ex_stack->cmdline);

    pe_app_param(path, raw, ex_pg_dir, ex_stack_page);
    return 0;
};

#define  master_tab    (page_tabs+ (page_tabs>>10))

void sys_app_entry(addr_t raw, addr_t ex_stack)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;

    size_t   img_size;
    count_t  img_pages;
    count_t  img_tabs;
    count_t  i;
    u32_t    tmp;

    __asm__ __volatile__ ("sti");

    DBG("pe_app_entry: raw %x esp %x\n", raw, ex_stack);

    dos = (PIMAGE_DOS_HEADER)raw;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    img_size  =  nt->OptionalHeader.SizeOfImage;

    img_pages = img_size >> 12;
    img_tabs  = ((img_size + 0x3FFFFF) & ~0x3FFFFF) >> 22;

    DBG("app pages %d app tabs %d\n", img_pages, img_tabs);

    for(i = 0; i < img_tabs; i++)
    {
        addr_t tab = core_alloc(0);
        ((u32_t*)master_tab)[i] = tab|7;                            /*   FIXME     */
    }

    ((u32_t*)master_tab)[0x7FC/4] = (ex_stack & 0xFFFFF000)|7;                                /*   FIXME     */

    __asm__ __volatile__ (
    "xorl %%eax, %%eax      \n\t"
    "rep stosl"
    :"=c"(tmp),"=D"(tmp)
    :"c"(img_tabs<<10),"D"(page_tabs)
    :"eax","cc");

    for(i = 0; i < img_pages; i++)
    {
        addr_t page = core_alloc(0);
        ((u32_t*)page_tabs)[i] = page | 7;                          /*   FIXME     */
    }

    create_image(0, raw);

    __asm__ __volatile__ (
    "xchgw %bx, %bx");

    addr_t entry = nt->OptionalHeader.AddressOfEntryPoint +
                   nt->OptionalHeader.ImageBase;

  //  __asm__ __volatile__ (
  //  "call %0":: "r" (entry));

    while(1);

};
