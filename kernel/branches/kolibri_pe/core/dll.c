
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>
#include <slab.h>
#include <pe.h>

#pragma pack(push,4)
typedef struct
{
  char     app_name[16];
  addr_t   fpu_state;                       /*      +16       */
  count_t  ev_count;                        /*      +20       */
  addr_t   fpu_handler;                     /*      +24       */
  addr_t   sse_handler;                     /*      +28       */
  addr_t   pl0_stack;                       /*      +32       */

  addr_t   heap_base;                       /*      +36       */
  addr_t   heap_top;                        /*      +40       */
  addr_t   cursor;                          /*      +44       */
  addr_t   fd_ev;                           /*      +48       */
  addr_t   bk_ev;                           /*      +52       */
  addr_t   fd_obj;                          /*      +56       */
  addr_t   bk_obj;                          /*      +60       */
  addr_t   saved_esp;                       /*      +64       */
  addr_t   io_map[2];                       /*      +68       */

  u32_t    dbg_state;                       /*      +76       */
  char    *cur_dir;                         /*      +80       */
  count_t  wait_timeout;                    /*      +84       */
  addr_t   saved_esp0;                      /*      +88       */

  link_t   dll_list;                        /*      +92       */

  u32_t    reserved0[7];                    /*      +100   db 28 dup(?)  */

  addr_t   wnd_shape;                       /*      +128      */
  u32_t    wnd_shape_scale;                 /*      +132      */
  u32_t    reserved1;                       /*      +136      */
  size_t   mem_size;                        /*      +140      */
}appdata_t;
#pragma pack(pop)


extern appdata_t *current_slot;

bool link_pe(addr_t img_base);

int __stdcall strncmp(const char *s1, const char *s2, size_t n);

extern int __stdcall mnt_exec(void *raw, size_t raw_size, char *path,
              char *cmdline, u32_t flags) asm ("mnt_exec");

dll_t core_dll;

slab_cache_t *dll_slab;

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

    dll_slab = slab_cache_create(sizeof(dll_t), 16,NULL,NULL,SLAB_CACHE_MAGDEFERRED);

    DBG("%s base %x size %x sections %d exports %x\n",
        core_dll.img_name, core_dll.img_base,
        core_dll.img_size, nt->FileHeader.NumberOfSections,
        core_dll.img_exp );
};


dll_t * find_dll(link_t *list, const char *name)
{
    dll_t* dll = (dll_t*)list;

    do
    {
        if( !strncmp(name,dll->img_name,16))
            return dll;

        dll = (dll_t*)dll->link.next;

    }while(&dll->link !=  list);

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


addr_t __fastcall pe_app_space(size_t size);

int __stdcall pe_app_param(char *path, void *raw, addr_t ex_pg_dir,
                          exec_stack_t *ex_stack) asm ("pe_app_param");

int sys_exec(char *path, char *cmdline, u32_t flags)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;

    size_t   img_size;
    count_t  img_pages;
    count_t  img_tabs;
    addr_t        ex_pg_dir;
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
        DBG("leagacy Kolibri application");
        int tmp =  mnt_exec(raw, raw_size, path, cmdline, flags);
        DBG("  pid %x\n",tmp);
        return tmp;
    }

    if( ! validate_pe(raw, raw_size, true) )
    {
        DBG("invalid executable file %s\n", path);
        mem_free(raw);
        return -31;
    }

    dos = (PIMAGE_DOS_HEADER)raw;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

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

    ex_stack_page  = core_alloc(0);                    /* 2^0 = 1 page   */
    if( ! ex_stack_page )
    {
        mem_free(raw);
        return -30;                                    /* FIXME          */
    };

    dos = (PIMAGE_DOS_HEADER)raw;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    img_size  =  nt->OptionalHeader.SizeOfImage;

    ex_pg_dir      = pe_app_space(img_size);

    if( !ex_pg_dir )
    {
        core_free(ex_stack_page);
        mem_free(raw);
        return -30;                                    /* FIXME          */
    };

    __asm__ __volatile__ (
    "xorl %%eax, %%eax      \n\t"
    "rep stosl"
    :"=c"(tmp),"=D"(tmp)
    :"c"(1024),"D"(ex_stack_page + OS_BASE)
    :"eax","cc");

    ex_stack = (exec_stack_t*)(ex_stack_page + OS_BASE
                               + PAGE_SIZE - stack_size);
    ex_stack->argc = 2;

    ex_path = MakePtr(char*, ex_stack, sizeof(exec_stack_t)+AUX_COUNT*sizeof(auxv_t));

    memcpy(ex_path, path, pathsize);
    ex_stack->path = (char*)(((addr_t)ex_path & 0xFFF) + 0x7FFFF000);  /* top of stack */

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

    return pe_app_param(path, raw, ex_pg_dir, ex_stack);
};


typedef struct
{
    u32_t edi;
    u32_t esi;
    u32_t ebp;
    u32_t esp;
    u32_t ebx;
    u32_t edx;
    u32_t ecx;
    u32_t eax;
    u32_t eip;
    u32_t cs;
    u32_t eflags;
    u32_t pe_sp;
    u32_t pe_ss;
}thr_stack_t;

void sys_app_entry(addr_t raw, thr_stack_t *thr_stack, exec_stack_t *ex_stack)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;

    size_t   img_size;
    count_t  img_pages;
    size_t   stack_size;
    addr_t   img_stack;
    addr_t  *pte;

    count_t  i;
    u32_t    tmp;

    __asm__ __volatile__ ("sti");

    dos = (PIMAGE_DOS_HEADER)raw;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    img_size  =  nt->OptionalHeader.SizeOfImage;

    current_slot->mem_size  = img_size;

    list_initialize(&current_slot->dll_list);

    pte = (addr_t*)page_tabs;
    img_pages = img_size >> 12;

    stack_size = (nt->OptionalHeader.SizeOfStackReserve + 4095) & ~4095;
    img_stack = 0x7FFFF000 - stack_size;
    stack_size>>= 12;

    while (img_pages--)
        *pte++ = 2;

    pte = &((addr_t*)page_tabs)[img_stack>>12];

    while(stack_size--)
        *pte++ = 0x02;

    addr_t stack_page = ((addr_t)ex_stack-OS_BASE) & ~4095;

    *pte = stack_page | 7;

    create_image(0, raw, false);

    init_user_heap();

    if (! link_pe(0))
    {
        DBG("\nunresolved imports\nexit\n");
        __asm__ __volatile__ (
        "int $0x40"::"a"(-1));
    };


 //   __asm__ __volatile__ (
 //   "xchgw %bx, %bx");

    addr_t entry = nt->OptionalHeader.AddressOfEntryPoint +
                   nt->OptionalHeader.ImageBase;

    thr_stack->edi = 0;
    thr_stack->esi = 0;
    thr_stack->ebp = 0;
    thr_stack->ebx = 0;
    thr_stack->edx = 0;
    thr_stack->ecx = 0;
    thr_stack->eax = 0;
    thr_stack->eip = entry;
    thr_stack->cs  = 0x1b;
    thr_stack->eflags = EFL_IOPL3 | EFL_IF;
    thr_stack->pe_sp = 0x7FFFF000 + ((u32_t)ex_stack & 0xFFF);
    thr_stack->pe_ss = 0x23;

};

void* __stdcall user_alloc(size_t size) asm("user_alloc");
void  __stdcall user_free(void *mem) asm("user_free");

dll_t* __fastcall load_dll(const char *path)
{
    PIMAGE_DOS_HEADER        dos;
    PIMAGE_NT_HEADERS32      nt;
    PIMAGE_EXPORT_DIRECTORY  exp;

    md_t    *img_md;

    size_t   img_size;
    addr_t   img_base;
    count_t  img_pages;

    size_t   raw_size = 0;
    void    *raw;

    DBG("\nload dll %s", path);

    raw = load_file(path, &raw_size);

    DBG("  raw = %x\n", raw);

    if( ! raw)
    {
        DBG("file not found: %s\n", path);
        return NULL;
    };

    if( ! validate_pe(raw, raw_size, false) )
    {
        DBG("invalid pe file %s\n", path);
        mem_free(raw);
        return NULL;
    }

    dos = (PIMAGE_DOS_HEADER)raw;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    img_size  =  nt->OptionalHeader.SizeOfImage;

    img_base = (addr_t)user_alloc(img_size);
    if( !img_base)
    {
        mem_free(raw);
        return NULL;
    };

    dll_t *dll = (dll_t*)slab_alloc(dll_slab,0);         /* FIXME check */
    if( !dll)
    {
        mem_free(raw);
        user_free((void*)img_base);
        return NULL;
    };

    create_image(img_base, (addr_t)raw, false);

    mem_free(raw);

    dos = (PIMAGE_DOS_HEADER)img_base;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);
    exp =  MakePtr(PIMAGE_EXPORT_DIRECTORY,img_base,
                   nt->OptionalHeader.DataDirectory[0].VirtualAddress);

    dll->img_base = img_base;
    dll->img_size = nt->OptionalHeader.SizeOfImage;
    dll->img_md   = NULL;

    dll->img_hdr  = nt;
    dll->img_sec  = MakePtr(PIMAGE_SECTION_HEADER,nt, sizeof(IMAGE_NT_HEADERS32));
    dll->img_exp  = MakePtr(PIMAGE_EXPORT_DIRECTORY,img_base,
                            nt->OptionalHeader.DataDirectory[0].VirtualAddress);
    dll->img_name = strupr(MakePtr(char*, img_base, exp->Name));

    list_insert(&current_slot->dll_list, &dll->link);

    return dll;
};

bool link_pe(addr_t img_base)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;
    char path[128];

    int warn = 0;

/* assumed that image is valid */

    dos = (PIMAGE_DOS_HEADER)img_base;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    if(nt->OptionalHeader.DataDirectory[1].Size)
    {
        PIMAGE_IMPORT_DESCRIPTOR imp;

        imp = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, img_base,
                      nt->OptionalHeader.DataDirectory[1].VirtualAddress);

        while ( 1 )
        {
            PIMAGE_THUNK_DATA32     thunk;

            PIMAGE_DOS_HEADER       expdos;
            PIMAGE_NT_HEADERS32     expnt;
            PIMAGE_EXPORT_DIRECTORY exp;

            u32_t   *iat;
            char    *libname;
            addr_t  *functions;
            u16_t   *ordinals;
            char   **funcname;

            dll_t   *exp_dll;

            if ( (imp->TimeDateStamp==0 ) && (imp->Name==0) )
                break;

            libname=MakePtr(char*,imp->Name, img_base);

            DBG("import from %s\n",libname);

            exp_dll = find_dll(&current_slot->dll_list, libname);
            if(exp_dll != NULL)
            {
                DBG("find %s\n", exp_dll->img_name);
            }
            else
            {
                int len = strlen(libname)+1;

                memcpy(path, "/sys/lib/",9);
                memcpy(&path[9],libname,len);

                exp_dll = load_dll(path);
                if( !exp_dll)
                {
                    DBG("can't load %s\n", path);
                    return false;
                };
            }

            exp = exp_dll->img_exp;

            functions = MakePtr(DWORD*,exp->AddressOfFunctions,exp_dll->img_base);
            ordinals = MakePtr(WORD*,  exp->AddressOfNameOrdinals,exp_dll->img_base);
            funcname = MakePtr(char**, exp->AddressOfNames,exp_dll->img_base);

            thunk = MakePtr(PIMAGE_THUNK_DATA32,
                            imp->Characteristics, img_base);
            iat= MakePtr(DWORD*,imp->FirstThunk, img_base);

            while ( 1 ) // Loop forever (or until we break out)
            {
                PIMAGE_IMPORT_BY_NAME ord;
                addr_t addr;
                *iat=0;

                if ( thunk->u1.AddressOfData == 0 )
                    break;

                if ( thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG )
                {
                   u16_t ordinal;
                   ordinal = thunk->u1.Ordinal & 0xFFFF;
                   *iat = functions[ordinal-exp->Base] + exp_dll->img_base;
                    break;
                }
                else
                {
                    ord = MakePtr(PIMAGE_IMPORT_BY_NAME,
                                  thunk->u1.AddressOfData, img_base);

                    DBG("import %s ", ord->Name);

                    if(strncmp(ord->Name,
                       MakePtr(char*,funcname[ord->Hint],exp_dll->img_base),32))
                    {
                        int ind;
                        char **names=funcname;

                        for(names = funcname,ind = 0;
                            ind < exp->NumberOfNames; names++,ind++)
                        {
                            if(!strncmp(ord->Name,MakePtr(char*,*names,exp_dll->img_base),32))
                            {
                                u16_t ordinal;
                                ordinal = ordinals[ind];
                                DBG("ordinal %d\t\tat %x\n", ordinal, functions[ordinal] + exp_dll->img_base);
                                *iat = functions[ordinal] + exp_dll->img_base;
                                break;
                            };
                        };
                        if(ind == exp->NumberOfNames)
                        {
                            DBG(" unresolved import %s\n",ord->Name);
                            warn=1;
                        };
                    }
                    else
                    {
                        DBG(" \tat %x\n", functions[ord->Hint] + exp_dll->img_base);
                        *iat = functions[ord->Hint] + exp_dll->img_base;
                    };
                };
                thunk++;            // Advance to next thunk
                iat++;
            }
            imp++;  // advance to next IMAGE_IMPORT_DESCRIPTOR
        };
    };

    if ( !warn )
        return true;
    else
        return false;
}

