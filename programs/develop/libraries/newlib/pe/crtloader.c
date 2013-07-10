#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <malloc.h>
#include <setjmp.h>
#include <envz.h>

#include <kos32sys.h>

#include "list.h"
#include "pe.h"

#define unlikely(x)     __builtin_expect(!!(x), 0)

//#define DBG(format,...) printf(format,##__VA_ARGS__)

#define DBG(format,...)

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

void* load_libc();

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

    img_base = user_alloc(nt->OptionalHeader.SizeOfImage);

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
        };
        printf("unmap base %p offset %x %d page(s)\n",
                img_base,
                nt->OptionalHeader.DataDirectory[5].VirtualAddress,
                (nt->OptionalHeader.DataDirectory[5].Size+4095)>>12);

        user_unmap(img_base,nt->OptionalHeader.DataDirectory[5].VirtualAddress,
                   nt->OptionalHeader.DataDirectory[5].Size);
    };
    return img_base;
};



void* load_libc()
{
    void     *raw_img;
    size_t    raw_size;
    void     *img_base = NULL;
    ufile_t   uf;

    uf = load_file("/kolibrios/lib/libc.dll");
    raw_img   = uf.data;
    raw_size  = uf.size;

    if(raw_img == NULL)
        return NULL;

    printf("libc.dll raw %p, size %d\n", raw_img, raw_size);

    if( validate_pe(raw_img, raw_size, 0) == 0)
    {
        printf("invalide libc.dll\n");
        user_free(raw_img);
    };

    img_base = create_image(raw_img);


    return img_base;

}


