
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <malloc.h>
#include <setjmp.h>
#include <envz.h>

#include <sys/ksys.h>

#include "list.h"
#include "pe.h"

#define unlikely(x)     __builtin_expect(!!(x), 0)

//#define DBG(format,...) printf(format,##__VA_ARGS__)

#define DBG(format,...)


void    init_loader(void *libc_image);
void*   create_image(void *raw);
int     link_image(void *img_base, PIMAGE_IMPORT_DESCRIPTOR imp);
void*   load_library(const char *name);

extern char* __appenv;
extern int   __appenv_size;

typedef struct tag_module module_t;

struct app_hdr
{
    char  banner[8];
    int   version;
    int   start;
    int   iend;
    int   memsize;
    int   stacktop;
    char  *cmdline;
    char  *path;
    int    reserved;
    void  *__idata_start;
    void  *__idata_end;
    void  (*main)(int argc, char **argv, char **envp);
};

struct tag_module
{
    struct list_head list;

    char       *img_name;
    char       *img_path;

    uint32_t    refcount;

    char       *start;
    char       *end;

    void       *entry;

    PIMAGE_NT_HEADERS32      img_hdr;
    PIMAGE_SECTION_HEADER    img_sec;
    PIMAGE_EXPORT_DIRECTORY  img_exp;
};

typedef struct
{
    struct list_head list;
    char *path;
    int   path_len;
}dll_path_t;


LIST_HEAD(path_list);

static module_t libc_dll;
static char libc_name[] = "libc.dll";
static char libc_path[] = "/KolibriOS/lib/libc.dll";

static inline int IsPowerOf2(uint32_t val)
{
    if(val == 0)
        return 0;
    return (val & (val - 1)) == 0;
}


int validate_pe(void *raw, size_t raw_size, int is_exec)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;

    dos = (PIMAGE_DOS_HEADER)raw;

    if( !raw || raw_size < sizeof(IMAGE_DOS_HEADER) )
        return 0;

    if( dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
        return 0;

    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    if( (uint32_t)nt < (uint32_t)raw)
        return 0;

    if(nt->Signature != IMAGE_NT_SIGNATURE)
        return 0;

    if(nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
        return 0;

    if(is_exec && (nt->FileHeader.Characteristics & IMAGE_FILE_DLL))
        return 0;

    if(nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        return 0;

    if( is_exec && nt->OptionalHeader.ImageBase != 0)
        return 0;

    if(nt->OptionalHeader.SectionAlignment < 4096)
    {
        if(nt->OptionalHeader.FileAlignment != nt->OptionalHeader.SectionAlignment)
            return 0;
    }
    else if(nt->OptionalHeader.SectionAlignment < nt->OptionalHeader.FileAlignment)
        return 0;

    if(!IsPowerOf2(nt->OptionalHeader.SectionAlignment) ||
       !IsPowerOf2(nt->OptionalHeader.FileAlignment))
        return 0;

    if(nt->FileHeader.NumberOfSections > 96)
        return 0;

    return 1;
}


void init_loader(void *libc_image)
{

    PIMAGE_DOS_HEADER        dos;
    PIMAGE_NT_HEADERS32      nt;
    PIMAGE_EXPORT_DIRECTORY  exp;

    struct app_hdr  *header = NULL;
    dll_path_t      *path;
    int             len;
    char            *p;

#if 0

    if(__appenv_size)
    {
        char *env;
        env = envz_get(__appenv, __appenv_size, "PATH");
        if( env )
        {
            while( *env )
            {
                p = env;
                while(*p)
                {
                    if( *p == 0x0D)
                        break;
                    else if( *p == 0x0A)
                        break;
                    else if( *p == ':')
                        break;
                    p++;
                };
                len = p-env;
                if(len)
                {
                    char *p1;

                    p1 = (char*)malloc(len+1);
                    memcpy(p1, env, len);
                    p1[len]=0;

                    path = (dll_path_t*)malloc(sizeof(dll_path_t));
                    INIT_LIST_HEAD(&path->list);
                    path->path = p1;
                    path->path_len = len;
                    DBG("add libraries path %s\n", path->path);
                    list_add_tail(&path->list, &path_list);
                };
                if(*p == ':')
                {
                    env = p+1;
                    continue;
                }
                else break;
            };
        };
    };
#endif

    len = strrchr(header->path, '/') - header->path+1;
    p = (char*)malloc(len+1);
    memcpy(p, header->path, len);
    p[len]=0;

    path = (dll_path_t*)malloc(sizeof(dll_path_t));
    INIT_LIST_HEAD(&path->list);
    path->path = p;
    path->path_len = len;
    DBG("add libraries path %s\n", path->path);
    list_add_tail(&path->list, &path_list);


    path = (dll_path_t*)malloc(sizeof(dll_path_t));
    INIT_LIST_HEAD(&path->list);
    path->path = "/kolibrios/lib/";
    path->path_len = 15;                           /* FIXME */
    DBG("add libraries path %s\n", path->path);
    list_add_tail(&path->list, &path_list);

    INIT_LIST_HEAD(&libc_dll.list);

    libc_dll.img_name = libc_name;
    libc_dll.img_path = libc_path;

    libc_dll.refcount = 1;

    dos =  (PIMAGE_DOS_HEADER)libc_image;
    nt  =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);
    exp =  MakePtr(PIMAGE_EXPORT_DIRECTORY, libc_image,
                   nt->OptionalHeader.DataDirectory[0].VirtualAddress);

    libc_dll.start = libc_image;
    libc_dll.end   = MakePtr(char*,libc_image, nt->OptionalHeader.SizeOfImage);

    libc_dll.img_hdr  = nt;
    libc_dll.img_sec  = MakePtr(PIMAGE_SECTION_HEADER,nt, sizeof(IMAGE_NT_HEADERS32));
    libc_dll.img_exp  = MakePtr(PIMAGE_EXPORT_DIRECTORY,libc_image,
                        nt->OptionalHeader.DataDirectory[0].VirtualAddress);

};

static inline void sec_copy(void *dst, void *src, size_t len)
{
    __asm__ __volatile__ (
    "shrl $2, %%ecx         \n\t"
    "rep movsl"
    :
    :"c"(len),"S"(src),"D"(dst)
    :"cc");
    __asm__ __volatile__ (
    ""
    :::"ecx","esi","edi");
};


void* create_image(void *raw)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;
    PIMAGE_SECTION_HEADER img_sec;

    void  *img_base;
    uint32_t  sec_align;
    int    i;

    dos = (PIMAGE_DOS_HEADER)raw;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    img_base = _ksys_alloc(nt->OptionalHeader.SizeOfImage);

    if(unlikely(img_base == NULL))
        return 0;

    sec_copy(img_base, raw, nt->OptionalHeader.SizeOfHeaders);

    img_sec = MakePtr(PIMAGE_SECTION_HEADER, nt, sizeof(IMAGE_NT_HEADERS32));

    sec_align = nt->OptionalHeader.SectionAlignment;

    for(i=0; i< nt->FileHeader.NumberOfSections; i++)
    {
        void *src_ptr;
        void *dest_ptr;
        size_t   sec_size;

        if ( img_sec->SizeOfRawData && img_sec->PointerToRawData )
        {
            src_ptr  = MakePtr(void*, raw, img_sec->PointerToRawData);
            dest_ptr = MakePtr(void*, img_base, img_sec->VirtualAddress);
            sec_copy(dest_ptr, src_ptr, img_sec->SizeOfRawData);
        };

        img_sec++;
    };

    if(nt->OptionalHeader.DataDirectory[5].Size)
    {
        PIMAGE_BASE_RELOCATION reloc;

        uint32_t delta = (uint32_t)img_base - nt->OptionalHeader.ImageBase;

        reloc = MakePtr(PIMAGE_BASE_RELOCATION, img_base,
                        nt->OptionalHeader.DataDirectory[5].VirtualAddress);

        while ( reloc->SizeOfBlock != 0 )
        {
            uint32_t  cnt;
            uint16_t *entry;
            uint16_t  reltype;
            uint32_t  offs;

            cnt = (reloc->SizeOfBlock - sizeof(*reloc))/sizeof(uint16_t);
            entry = MakePtr( uint16_t*, reloc, sizeof(*reloc) );

            for ( i=0; i < cnt; i++ )
            {
                uint16_t *p16;
                uint32_t *p32;

                reltype = (*entry & 0xF000) >> 12;
                offs = (*entry & 0x0FFF) + reloc->VirtualAddress;
                switch(reltype)
                {
                    case 1:
                        p16 = MakePtr(uint16_t*, img_base, offs);
                        *p16+= (uint16_t)(delta>>16);
                        break;
                    case 2:
                        p16 = MakePtr(uint16_t*, img_base, offs);
                        *p16+= (uint16_t)delta;
                        break;
                    case 3:
                        p32 = MakePtr(uint32_t*, img_base, offs);
                        *p32+= delta;
                }
                entry++;
            }
            reloc = MakePtr(PIMAGE_BASE_RELOCATION, reloc,reloc->SizeOfBlock);
        }
    };
    return img_base;
};


int link_image(void *img_base, PIMAGE_IMPORT_DESCRIPTOR imp)
{
    static jmp_buf loader_env;
    static int recursion = -1;
    int warn = 0;

    recursion++;
    if( !recursion )
    {
        if( unlikely(setjmp(loader_env) != 0))
        {
            recursion = -1;
            return 0;
        };
    };

    while ( imp->Name )
    {
        PIMAGE_DOS_HEADER        expdos;
        PIMAGE_NT_HEADERS32      expnt;
        PIMAGE_EXPORT_DIRECTORY  exp;
        PIMAGE_THUNK_DATA32      thunk;

        void       **iat;
        char       *libname;
        uint32_t   *exp_functions;
        uint16_t   *exp_ordinals;
        char      **exp_names;

        const module_t *api;

        libname=MakePtr(char*,imp->Name, img_base);

        DBG("import from %s\n",libname);

        api = load_library(libname);
        if(unlikely(api == NULL))
        {
            printf("library %s not found\n", libname);
            longjmp(loader_env, 1);
        }

        iat = MakePtr(void**,imp->FirstThunk, img_base);

        if(imp->OriginalFirstThunk !=0 )
        {
            thunk = MakePtr(PIMAGE_THUNK_DATA32,imp->OriginalFirstThunk, img_base);
        }
        else
        {
            thunk = MakePtr(PIMAGE_THUNK_DATA32,imp->FirstThunk, img_base);
        };

        exp = api->img_exp;

        exp_functions = MakePtr(uint32_t*,exp->AddressOfFunctions,api->start);
        exp_ordinals = MakePtr(uint16_t*,  exp->AddressOfNameOrdinals,api->start);
        exp_names = MakePtr(char**, exp->AddressOfNames,api->start);

        while ( thunk->u1.AddressOfData != 0 )
        {
            PIMAGE_IMPORT_BY_NAME imp_name;

            if (thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
            {
//                ordinal = (*func_list) & 0x7fffffff;
//               *ImportAddressList = LdrGetExportByOrdinal(ImportedModule->DllBase, Ordinal);
//                if ((*ImportAddressList) == NULL)
//                {
//                    DPRINT1("Failed to import #%ld from %wZ\n", Ordinal, &ImportedModule->FullDllName);
//                    RtlpRaiseImportNotFound(NULL, Ordinal, &ImportedModule->FullDllName);
//                    return STATUS_ENTRYPOINT_NOT_FOUND;
//                }
            }
            else
            {
                char *export_name;
                uint16_t   ordinal;
                void      *function;
                uint32_t   minn;
                uint32_t   maxn;

                imp_name = MakePtr(PIMAGE_IMPORT_BY_NAME,
                              thunk->u1.AddressOfData, img_base);
                *iat = NULL;

                DBG("import %s", imp_name->Name);

                if(imp_name->Hint < exp->NumberOfNames)
                {
                    export_name = MakePtr(char*,exp_names[imp_name->Hint],
                                          api->start);
                    if(strcmp(imp_name->Name, export_name) == 0)
                    {
                        ordinal = exp_ordinals[imp_name->Hint];
                        function = MakePtr(void*,exp_functions[ordinal], api->start);
                        if((uint32_t)function >= (uint32_t)exp)
                        {
                            printf("forward %s\n", function);
                            warn=1;
                        }
                        else
                        {
                            DBG(" \t\tat %x\n", function);
                            *iat = function;
                        };
                        thunk++;  // Advance to next thunk
                        iat++;
                        continue;
                    };
                };

                minn = 0;
                maxn = exp->NumberOfNames - 1;
                while (minn <= maxn)
                {
                    int mid;
                    int res;

                    mid = (minn + maxn) / 2;

                    export_name = MakePtr(char*,exp_names[mid],api->start);

                    res = strcmp(export_name, imp_name->Name);
                    if (res == 0)
                    {
                        ordinal  = exp_ordinals[mid];
                        function = MakePtr(void*,exp_functions[ordinal], api->start);

                        if((uint32_t)function >= (uint32_t)exp)
                        {
                            printf("forward %s\n", function);
                            warn=1;
                        }
                        else
                        {
                            DBG(" \t\tat %x\n", function);
                            *iat = function;
                        };
                        break;
                    }
                    else if (minn == maxn)
                    {
                        printf(" unresolved %s\n",imp_name->Name);
                        warn=1;
                        break;
                    }
                    else if (res > 0)
                    {
                        maxn = mid - 1;
                    }
                    else
                    {
                        minn = mid + 1;
                    }
                };
            };
            thunk++;            // Advance to next thunk
            iat++;
        }
        imp++;  // advance to next IMAGE_IMPORT_DESCRIPTOR
    };

    recursion--;

    if ( !warn )
        return 1;
    else
        return 0;
}

static void send_dbg_msg(void *msg)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(69),
      "b"(10),
      "S"(msg));
};


int link_app()
{
    struct app_hdr *header = NULL;
    PIMAGE_IMPORT_DESCRIPTOR imp;

    imp = (PIMAGE_IMPORT_DESCRIPTOR)header->__idata_start;

    if(link_image(NULL, imp))
    {

#ifdef DEBUG_INFO
        struct
        {
            void     *start;
            uint32_t  end;
            char      name[24];
        } dbg_msg;

        module_t *mod = &libc_dll;

        do
        {
            printf("%s %x - %x\n",
                   mod->img_name, mod->start, mod->end);

//            asm volatile("int3");

            dbg_msg.start = mod->start;
            dbg_msg.end   = mod->end;
            strcpy(dbg_msg.name, mod->img_name);
            send_dbg_msg(&dbg_msg);
            mod = (module_t*)mod->list.next;
        }while(mod != &libc_dll);
#endif

        return 1;
    };

    return 0;
}


void* get_entry_point(void *raw)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;

    dos = (PIMAGE_DOS_HEADER)raw;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    return  MakePtr(void*, raw, nt->OptionalHeader.AddressOfEntryPoint);
};


void *get_proc_address(void *handle, const char *proc_name)
{

    module_t *module = handle;
    PIMAGE_DOS_HEADER        expdos;
    PIMAGE_NT_HEADERS32      expnt;
    PIMAGE_EXPORT_DIRECTORY  exp;

    uint32_t   *exp_functions;
    uint16_t   *exp_ordinals;
    char      **exp_names;

    int minn, maxn;
    char *export_name;
    uint16_t   ordinal;
    void *function=NULL;

    exp = module->img_exp;

    exp_functions = MakePtr(uint32_t*,exp->AddressOfFunctions,module->start);
    exp_ordinals = MakePtr(uint16_t*,  exp->AddressOfNameOrdinals,module->start);
    exp_names = MakePtr(char**, exp->AddressOfNames,module->start);

    minn = 0;
    maxn = exp->NumberOfNames - 1;
    while (minn <= maxn)
    {
        int mid;
        int res;

        mid = (minn + maxn) / 2;

        export_name = MakePtr(char*,exp_names[mid],module->start);

        res = strcmp(export_name, proc_name);
        if (res == 0)
        {
            ordinal  = exp_ordinals[mid];
            function = MakePtr(void*,exp_functions[ordinal], module->start);

            if((uint32_t)function >= (uint32_t)exp)
            {
                printf("forward %s\n", function);
            }
            else
            {
                DBG(" \t\tat %x\n", function);
            };
            break;
        }
        else if (minn == maxn)
        {
            DBG(" unresolved %s\n",proc_name);
            break;
        }
        else if (res > 0)
        {
            maxn = mid - 1;
        }
        else
        {
            minn = mid + 1;
        }
    };

    return function;
};

static void *load_lib_internal(const char *path)
{
    PIMAGE_DOS_HEADER        dos;
    PIMAGE_NT_HEADERS32      nt;
    PIMAGE_EXPORT_DIRECTORY  exp;

    ksys_ufile_t   uf;
    void     *raw_img;
    size_t    raw_size;
    void     *img_base = NULL;

    uf = _ksys_load_file(path);
    raw_img  = uf.data;
    raw_size = uf.size;

    if(raw_img == NULL)
        return NULL;

    if( validate_pe(raw_img, raw_size, 0) == 0)
    {
        printf("invalide module %s\n", path);
        _ksys_free(raw_img);
        return NULL;
    };

    img_base = create_image(raw_img);
    _ksys_free(raw_img);

    if( unlikely(img_base == NULL) )
        printf("cannot create image %s\n",path);

    return img_base;
}

void* load_library(const char *name)
{
    PIMAGE_DOS_HEADER        dos;
    PIMAGE_NT_HEADERS32      nt;
    PIMAGE_EXPORT_DIRECTORY  exp;

    module_t    *module, *mod = &libc_dll;
    dll_path_t  *dllpath;
    char        *path;
    int          len;
    char        *libname, *tmp;
    void        *img_base;


/*  check for already loaded libraries  */

    tmp = strrchr(name, '/');
    libname = path = tmp != NULL ? tmp+1 : (char*)name;

//    printf("path %s\n", path);

    do
    {
        if( !strncmp(path, mod->img_name, 16))
            return mod;
        mod = (module_t*)mod->list.next;
    }while(mod != &libc_dll);

    if(name[0] == '/')
    {
        path = (char*)name;
        img_base = load_lib_internal(path);
    }
    else
    {
        len = strlen(libname);
        list_for_each_entry(dllpath, &path_list, list)
        {
            path = alloca(len+dllpath->path_len+1);
            memcpy(path, dllpath->path, dllpath->path_len);

            memcpy(path+dllpath->path_len, libname, len);
            path[len+dllpath->path_len]=0;

//            printf("%s\n", path);

            img_base = load_lib_internal(path);

            if( unlikely(img_base == NULL) )
                continue;
        };
    }

    if( unlikely(img_base == NULL) )
    {
        printf("unable to load %s\n", name);
        return 0;
    };

    module = malloc(sizeof(module_t));

    if(unlikely(module == NULL))
    {
        printf("%s epic fail: no enough memory\n",__FUNCTION__);
        goto err1;
    }

    INIT_LIST_HEAD(&module->list);

    module->img_name = strdup(libname);
    module->img_path = strdup(path);
    module->start    = img_base;
    module->entry    = get_entry_point(img_base);
    module->refcount = 1;

    dos =  (PIMAGE_DOS_HEADER)img_base;
    nt  =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);
    exp =  MakePtr(PIMAGE_EXPORT_DIRECTORY, img_base,
               nt->OptionalHeader.DataDirectory[0].VirtualAddress);

    module->end   = MakePtr(char*,img_base, nt->OptionalHeader.SizeOfImage);

    module->img_hdr  = nt;
    module->img_sec  = MakePtr(PIMAGE_SECTION_HEADER,nt, sizeof(IMAGE_NT_HEADERS32));
    module->img_exp  = MakePtr(PIMAGE_EXPORT_DIRECTORY, img_base,
                       nt->OptionalHeader.DataDirectory[0].VirtualAddress);

    list_add_tail(&module->list, &libc_dll.list);

    if(nt->OptionalHeader.DataDirectory[1].Size)
    {
        PIMAGE_IMPORT_DESCRIPTOR imp;
        int (*dll_startup)(module_t *mod, uint32_t reason);

        imp = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, img_base,
                    nt->OptionalHeader.DataDirectory[1].VirtualAddress);

        if(link_image(img_base, imp) == 0)
            goto err2;

        dll_startup = get_proc_address(module, "DllStartup");
        if( dll_startup )
        {
            if( 0 == dll_startup(module, 1))
                goto err2;
        }
    };

//    printf("module %s %p - %p\n", name, module->start, module->end);

    return module;

err2:
    list_del(&module->list);
    free(module->img_name);
    free(module->img_path);
    free(module);
err1:
    _ksys_free(img_base);
    return NULL;
};

void enumerate_libraries(int (*callback)(void *handle, const char* name,
                                         uint32_t base, uint32_t size, void *user_data),
                         void *user_data)
{
    module_t *mod = &libc_dll;

    do
    {
        if(0 == callback(mod, mod->img_name, (uint32_t)mod->start,
                         mod->end - mod->start, user_data))
            break;
        mod = (module_t*)mod->list.next;
    }while(mod != &libc_dll);
}
