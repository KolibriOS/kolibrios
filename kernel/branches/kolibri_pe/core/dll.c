
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>
#include <slab.h>
#include <pe.h>

int __stdcall strncmp(const char *s1, const char *s2, size_t n);

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

