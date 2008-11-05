
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>
#include <slab.h>
#include <pe.h>


static inline bool IsPowerOf2(u32_t val)
{
    if(val == 0)
        return false;
    return (val & (val - 1)) == 0;
}


static inline void sec_copy(addr_t dst, addr_t src, size_t len)
{
    u32_t tmp;
    __asm__ __volatile__ (
    "shrl $2, %%ecx         \n\t"
    "rep movsl"
    :"=c"(tmp),"=S"(tmp),"=D"(tmp)
    :"c"(len),"S"(src),"D"(dst)
    :"cc");
};

static inline void sec_clear(addr_t dst, size_t len)
{
    u32_t tmp;
    __asm__ __volatile__ (
    "xorl %%eax, %%eax      \n\t"
    "rep stosb"
    :"=c"(tmp),"=D"(tmp)
    :"c"(len),"D"(dst)
    :"eax","cc");
};

int __stdcall strncmp(const char *s1, const char *s2, size_t n);

void __export create_image(addr_t img_base, addr_t raw) asm ("CreateImage");
bool link_image(addr_t img_base);

md_t* __fastcall load_image(const char *path);

/*
void* __fastcall load_pe(const char *path)
{
    md_t  *md;

    md = load_image(path);

    if( md )
        return (void*)md->base;

    return NULL;
};
*/

bool validate_pe(void *raw, size_t raw_size)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;

    dos = (PIMAGE_DOS_HEADER)raw;

    if( !raw || raw_size < sizeof(IMAGE_DOS_HEADER) )
        return false;

    if( dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
        return false;

    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    if( (addr_t)nt < (addr_t)raw)
        return false;

    if(nt->Signature != IMAGE_NT_SIGNATURE)
        return false;

    if(nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        return false;

    if(nt->OptionalHeader.SectionAlignment < PAGE_SIZE)
	{
        if(nt->OptionalHeader.FileAlignment != nt->OptionalHeader.SectionAlignment)
            return false;
	}
    else if(nt->OptionalHeader.SectionAlignment < nt->OptionalHeader.FileAlignment)
        return false;

    if(!IsPowerOf2(nt->OptionalHeader.SectionAlignment) ||
       !IsPowerOf2(nt->OptionalHeader.FileAlignment))
        return false;

    if(nt->FileHeader.NumberOfSections > 96)
        return false;

    return true;
}

md_t* __fastcall load_image(const char *path)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;

    md_t    *img_md;

    size_t   img_size;
    addr_t   img_base;
    count_t  img_pages;

    size_t   raw_size = 0;
    void    *raw;

    DBG("\nload image %s", path);

    raw = load_file(path, &raw_size);

    DBG("  raw = %x\n", raw);

    if( ! raw)
    {
        DBG("file not found: %s\n", path);
        return NULL;
    };

    if( ! validate_pe(raw, raw_size) )
	{
        DBG("invalid pe file %s\n", path);
        mem_free(raw);
            return NULL;
	}

    dos = (PIMAGE_DOS_HEADER)raw;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    img_size  =  nt->OptionalHeader.SizeOfImage;

    img_md  = md_alloc(img_size, PG_SW);


    if( !img_md)
    {
        mem_free(raw);
        return NULL;
    };

    img_base = img_md->base;

    create_image(img_base, (addr_t)raw);

    mem_free(raw);

//    dos = (PIMAGE_DOS_HEADER)img_base;
//    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    return img_md;
};


/*
addr_t get_proc_addr(addr_t module, char *name)
{
    PIMAGE_DOS_HEADER  expdos;
    PIMAGE_NT_HEADERS32  expnt;
    PIMAGE_EXPORT_DIRECTORY exp;
    u32_t *functions;
    char **funcname;
    int ind;

    expdos = (PIMAGE_DOS_HEADER)module;
    expnt =  MakePtr( PIMAGE_NT_HEADERS32, expdos, expdos->e_lfanew);

    exp = MakePtr(PIMAGE_EXPORT_DIRECTORY,module,
                  expnt->OptionalHeader.DataDirectory[0].VirtualAddress);

    functions = MakePtr(DWORD*,exp->AddressOfFunctions,module);
    funcname = MakePtr(char**,exp->AddressOfNames,module);

    for(ind=0; *funcname;funcname++,ind++)
    {
        if(!strcmp(name,MakePtr(char*,*funcname,module)))
            return functions[ind] + module;
    };
    return -1;
};
*/


void create_image(addr_t img_base, addr_t raw)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;
    PIMAGE_SECTION_HEADER img_sec;

    u32_t  sec_align;
    int    i;


/* assumed that image is valid */

    dos = (PIMAGE_DOS_HEADER)raw;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    sec_copy(img_base, raw, nt->OptionalHeader.SizeOfHeaders);

    img_sec = MakePtr(PIMAGE_SECTION_HEADER, nt, sizeof(IMAGE_NT_HEADERS32));

    sec_align = nt->OptionalHeader.SectionAlignment;

    for(i=0; i< nt->FileHeader.NumberOfSections; i++)
    {
        addr_t src_ptr;
        addr_t dest_ptr;
        size_t sec_size;

        src_ptr = MakePtr(addr_t, raw, img_sec->PointerToRawData);
        dest_ptr = MakePtr(addr_t,img_base, img_sec->VirtualAddress);

        if(img_sec->SizeOfRawData)
            sec_copy(dest_ptr, src_ptr, img_sec->SizeOfRawData);

        sec_size = (img_sec->Misc.VirtualSize + sec_align -1) & -sec_align;

        if(sec_size > img_sec->SizeOfRawData)
            sec_clear(dest_ptr + img_sec->SizeOfRawData,
                      sec_size - img_sec->SizeOfRawData);
        img_sec++;
    }

    if(nt->OptionalHeader.DataDirectory[5].Size)
    {
        PIMAGE_BASE_RELOCATION reloc;

/* FIXME addr_t */

        u32_t delta = (u32_t)img_base - nt->OptionalHeader.ImageBase;

        reloc = MakePtr(PIMAGE_BASE_RELOCATION, img_base,
                        nt->OptionalHeader.DataDirectory[5].VirtualAddress);

        while ( reloc->SizeOfBlock != 0 )
        {
            u32_t  cnt;
            u16_t *entry;
            u16_t  reltype;
            u32_t  offs;

            cnt = (reloc->SizeOfBlock - sizeof(*reloc))/sizeof(u16_t);
            entry = MakePtr( u16_t*, reloc, sizeof(*reloc) );

            for ( i=0; i < cnt; i++ )
            {
                u16_t *p16;
                u32_t *p32;

                reltype = (*entry & 0xF000) >> 12;
                offs = (*entry & 0x0FFF) + reloc->VirtualAddress;
                switch(reltype)
                {
                    case 1:
                        p16 = MakePtr(u16_t*, img_base, offs);
                        *p16+= (u16_t)(delta>>16);
                        break;
                    case 2:
                        p16 = MakePtr(u16_t*, img_base, offs);
                        *p16+= (u16_t)delta;
                        break;
                    case 3:
                        p32 = MakePtr(u32_t*, img_base, offs);
                        *p32+= delta;
                }
                entry++;
            }
            reloc = MakePtr(PIMAGE_BASE_RELOCATION, reloc,reloc->SizeOfBlock);
        }
    };

    DBG("\ncreate pe base %x, size %x, %d sections\n\n",img_base,
         nt->OptionalHeader.SizeOfImage, nt->FileHeader.NumberOfSections);
};


bool link_image(addr_t img_base)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;

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

            exp_dll = find_dll(&core_dll.link, libname);
            if(exp_dll != NULL)
            {
                DBG("find %s\n", exp_dll->img_name);
            }
            else
            {
                DBG("can't find %s\n", libname);
                return false;
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

                if ( thunk->u1.AddressOfData == 0 )
                    break;

                if ( thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG )
                {
        //  printf("  %4u\n", thunk->u1.Ordinal & 0xFFFF);
                    break;
                }
                else
                {
                    ord = MakePtr(PIMAGE_IMPORT_BY_NAME,
                                  thunk->u1.AddressOfData, img_base);
                    *iat=0;

                    DBG("import %s", ord->Name);

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
                                DBG(" \t\tat %x\n", functions[ordinal] + exp_dll->img_base);
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


