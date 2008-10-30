
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>
#include <slab.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
typedef unsigned char BYTE;

#define IMAGE_DOS_SIGNATURE  0x5A4D
#define IMAGE_NT_SIGNATURE   0x00004550
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10b

#pragma pack(push,2)
typedef struct _IMAGE_DOS_HEADER
{
    WORD    e_magic;
    WORD    e_cblp;
    WORD    e_cp;
    WORD    e_crlc;
    WORD    e_cparhdr;
    WORD    e_minalloc;
    WORD    e_maxalloc;
    WORD    e_ss;
    WORD    e_sp;
    WORD    e_csum;
    WORD    e_ip;
    WORD    e_cs;
    WORD    e_lfarlc;
    WORD    e_ovno;
    WORD    e_res[4];
    WORD    e_oemid;
    WORD    e_oeminfo;
    WORD    e_res2[10];
    LONG    e_lfanew;
} IMAGE_DOS_HEADER,*PIMAGE_DOS_HEADER;
#pragma pack(pop)


#pragma pack(push,4)
typedef struct _IMAGE_FILE_HEADER
{
    WORD    Machine;
    WORD    NumberOfSections;
    DWORD   TimeDateStamp;
    DWORD   PointerToSymbolTable;
    DWORD   NumberOfSymbols;
    WORD    SizeOfOptionalHeader;
    WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD   VirtualAddress;
    DWORD   Size;
} IMAGE_DATA_DIRECTORY,*PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

typedef struct _IMAGE_OPTIONAL_HEADER {
    WORD    Magic;
    BYTE    MajorLinkerVersion;
    BYTE    MinorLinkerVersion;
    DWORD   SizeOfCode;
    DWORD   SizeOfInitializedData;
    DWORD   SizeOfUninitializedData;
    DWORD   AddressOfEntryPoint;
    DWORD   BaseOfCode;
    DWORD   BaseOfData;
    DWORD   ImageBase;
    DWORD   SectionAlignment;
    DWORD   FileAlignment;
    WORD    MajorOperatingSystemVersion;
    WORD    MinorOperatingSystemVersion;
    WORD    MajorImageVersion;
    WORD    MinorImageVersion;
    WORD    MajorSubsystemVersion;
    WORD    MinorSubsystemVersion;
    DWORD   Win32VersionValue;
    DWORD   SizeOfImage;
    DWORD   SizeOfHeaders;
    DWORD   CheckSum;
    WORD    Subsystem;
    WORD    DllCharacteristics;
    DWORD   SizeOfStackReserve;
    DWORD   SizeOfStackCommit;
    DWORD   SizeOfHeapReserve;
    DWORD   SizeOfHeapCommit;
    DWORD   LoaderFlags;
    DWORD   NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER,*PIMAGE_OPTIONAL_HEADER;

#pragma pack(pop)


#pragma pack(push,4)
typedef struct _IMAGE_NT_HEADERS
{
    DWORD Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS32,*PIMAGE_NT_HEADERS32;

#define IMAGE_SIZEOF_SHORT_NAME    8

typedef struct _IMAGE_SECTION_HEADER
{
	BYTE Name[IMAGE_SIZEOF_SHORT_NAME];
    union
    {
        DWORD PhysicalAddress;
		DWORD VirtualSize;
	} Misc;
    DWORD   VirtualAddress;
    DWORD   SizeOfRawData;
    DWORD   PointerToRawData;
    DWORD   PointerToRelocations;
    DWORD   PointerToLinenumbers;
    WORD    NumberOfRelocations;
    WORD    NumberOfLinenumbers;
    DWORD   Characteristics;
} IMAGE_SECTION_HEADER,*PIMAGE_SECTION_HEADER;
#pragma pack(pop)

#pragma pack(push,4)
typedef struct _IMAGE_BASE_RELOCATION {
	DWORD VirtualAddress;
	DWORD SizeOfBlock;
} IMAGE_BASE_RELOCATION,*PIMAGE_BASE_RELOCATION;
#pragma pack(pop)

typedef struct _IMAGE_IMPORT_DESCRIPTOR
{
    union
    {
		DWORD Characteristics;
		DWORD OriginalFirstThunk;
    };
    DWORD   TimeDateStamp;
    DWORD   ForwarderChain;
    DWORD   Name;
    DWORD   FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR,*PIMAGE_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_THUNK_DATA32
{
    union
    {
		DWORD ForwarderString;
		DWORD Function;
		DWORD Ordinal;
		DWORD AddressOfData;
	} u1;
} IMAGE_THUNK_DATA32,*PIMAGE_THUNK_DATA32;

typedef struct _IMAGE_IMPORT_BY_NAME
{
	WORD Hint;
	BYTE Name[1];
} IMAGE_IMPORT_BY_NAME,*PIMAGE_IMPORT_BY_NAME;

#define IMAGE_ORDINAL_FLAG 0x80000000

typedef struct _IMAGE_EXPORT_DIRECTORY {
	DWORD Characteristics;
	DWORD TimeDateStamp;
	WORD MajorVersion;
	WORD MinorVersion;
	DWORD Name;
	DWORD Base;
	DWORD NumberOfFunctions;
	DWORD NumberOfNames;
	DWORD AddressOfFunctions;
	DWORD AddressOfNames;
	DWORD AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY,*PIMAGE_EXPORT_DIRECTORY;

//extern  IMAGE_EXPORT_DIRECTORY kernel_exports;

#define MakePtr( cast, ptr, addValue ) (cast)( (addr_t)(ptr) + (addValue) )

typedef struct
{
    addr_t  base;
    addr_t  frame;
    md_t    *md;

    IMAGE_OPTIONAL_HEADER  *opthdr;

}dll_t;

static inline bool IsPowerOf2(u32_t val)
{
    if(val == 0)
        return false;
    return (val & (val - 1)) == 0;
}


static inline void sec_copy(void *dst, const void *src, size_t len)
{
    u32_t tmp;
    __asm__ __volatile__ (
    "shrl $2, %%ecx         \n\t"
    "rep movsl"
    :"=c"(tmp),"=S"(tmp),"=D"(tmp)
    :"c"(len),"S"(src),"D"(dst)
    :"cc");
};

static inline void sec_clear(void *dst, size_t len)
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


void __export create_image(void *img_base, void *image) asm ("CreateImage");

md_t* __fastcall load_image(const char *path);


void* __fastcall load_pe(const char *path)
{
    md_t  *md;

    md = load_image(path);

    if( md )
        return (void*)md->base;

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

    dos = (PIMAGE_DOS_HEADER)md->base;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    drv_entry = MakePtr(drv_entry_t*, md->base,
                        nt->OptionalHeader.AddressOfEntryPoint);

    srv = drv_entry(1);

    if(srv != NULL)
       srv->entry = nt->OptionalHeader.AddressOfEntryPoint + md->base;

    return srv;
}

md_t* __fastcall load_image(const char *path)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;

    md_t    *img_md;

    size_t   img_size;
    void    *img_base;
    count_t  img_pages;

    size_t   raw_size = 0;
    void    *raw;

//    void    *image;

    DBG("load file %s\n", path);

    raw = load_file(path, &raw_size);

    DBG("raw = %x\n\n", raw);

    dos = (PIMAGE_DOS_HEADER)raw;

    if( !raw || raw_size < sizeof(IMAGE_DOS_HEADER) )
        return NULL;

    if( dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
        return NULL;

    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    if( (addr_t)nt < (addr_t)raw)
        return NULL;

    if(nt->Signature != IMAGE_NT_SIGNATURE)
        return NULL;

    if(nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        return NULL;

    if(nt->OptionalHeader.SectionAlignment < PAGE_SIZE)
	{
        if(nt->OptionalHeader.FileAlignment != nt->OptionalHeader.SectionAlignment)
            return NULL;
	}
    else if(nt->OptionalHeader.SectionAlignment < nt->OptionalHeader.FileAlignment)
        return NULL;

    if(!IsPowerOf2(nt->OptionalHeader.SectionAlignment) ||
       !IsPowerOf2(nt->OptionalHeader.FileAlignment))
        return NULL;

    if(nt->FileHeader.NumberOfSections > 96)
        return NULL;

    img_size  =  nt->OptionalHeader.SizeOfImage;
//    img_pages = img_size / PAGE_SIZE;

    img_md  = md_alloc(img_size, PG_SW);


    if( !img_md)
    {
        mem_free(raw);
        return NULL;
    };

    img_base = (void*)img_md->base;

    create_image(img_base, raw);

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


void create_image(void *img_base, void *image)
{
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS32   nt;
    PIMAGE_SECTION_HEADER img_sec;

    u32_t  sec_align;
    int    i;


/* assumed that image is valid */

    dos = (PIMAGE_DOS_HEADER)image;
    nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);

    sec_copy(img_base,image,nt->OptionalHeader.SizeOfHeaders);

    img_sec = MakePtr(PIMAGE_SECTION_HEADER,nt,sizeof(IMAGE_NT_HEADERS32));

    sec_align = nt->OptionalHeader.SectionAlignment;

    for(i=0; i< nt->FileHeader.NumberOfSections; i++)
    {
        char *src_ptr;
        char *dest_ptr;
        size_t sec_size;

        src_ptr = MakePtr(char*, image, img_sec->PointerToRawData);
        dest_ptr = MakePtr(char*,img_base, img_sec->VirtualAddress);

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

    if(nt->OptionalHeader.DataDirectory[1].Size)
    {
        PIMAGE_IMPORT_DESCRIPTOR imp;

        int warn = 0;

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


            if ( (imp->TimeDateStamp==0 ) && (imp->Name==0) )
                break;

            libname=MakePtr(char*,imp->Name, img_base);

            DBG("import from %s\n",libname);

            expdos = (PIMAGE_DOS_HEADER)IMAGE_BASE;
            expnt =  MakePtr( PIMAGE_NT_HEADERS32, expdos, expdos->e_lfanew);

            exp = MakePtr(PIMAGE_EXPORT_DIRECTORY,LOAD_BASE,
                    expnt->OptionalHeader.DataDirectory[0].VirtualAddress);

            functions = MakePtr(DWORD*,exp->AddressOfFunctions,LOAD_BASE);
            ordinals = MakePtr(WORD*,  exp->AddressOfNameOrdinals,LOAD_BASE);
            funcname = MakePtr(char**, exp->AddressOfNames,LOAD_BASE);

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
                       MakePtr(char*,funcname[ord->Hint],LOAD_BASE),32))
                    {
                        int ind;
                        char **names=funcname;

                        for(names = funcname,ind = 0;
                            ind < exp->NumberOfNames; names++,ind++)
                        {
                            if(!strncmp(ord->Name,MakePtr(char*,*names,LOAD_BASE),32))
                            {
                                DBG(" \tat %x\n", functions[ind] + LOAD_BASE);
                                *iat = functions[ind] + LOAD_BASE;
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
                        DBG(" \tat %x\n", functions[ord->Hint] + LOAD_BASE);
                        *iat = functions[ord->Hint] + LOAD_BASE;
                    };
                };
                thunk++;            // Advance to next thunk
                iat++;
            }
            imp++;  // advance to next IMAGE_IMPORT_DESCRIPTOR
        };
    };

    DBG("\ncreate pe base %x, size %x, %d sections\n\n",img_base,
         nt->OptionalHeader.SizeOfImage, nt->FileHeader.NumberOfSections);
};





/*

u32 map_PE(u32 base, void *image)
{
  PIMAGE_DOS_HEADER dos;
  PIMAGE_NT_HEADERS32 nt;
  PIMAGE_SECTION_HEADER sec;

  int i;
  int pages;

  dos = (PIMAGE_DOS_HEADER)image;
  nt =  MakePtr( PIMAGE_NT_HEADERS32, dos, dos->e_lfanew);


    img_size  =  nt->OptionalHeader.SizeOfImage;
    img_pages = img_size / PAGE_SIZE;

    img_md  = md_alloc(img_size, PG_SW);

    if( !img_md)
        return NULL;



  scopy(base,(u32)image,nt->OptionalHeader.SizeOfHeaders);

  sec = MakePtr(PIMAGE_SECTION_HEADER,nt,sizeof(IMAGE_NT_HEADERS32));


  if(nt->OptionalHeader.DataDirectory[1].Size)
  {
    PIMAGE_IMPORT_DESCRIPTOR imp;

    imp = MakePtr(PIMAGE_IMPORT_DESCRIPTOR,base,
                  nt->OptionalHeader.DataDirectory[1].VirtualAddress);
    while ( 1 )
    {
      PIMAGE_THUNK_DATA32 thunk;
      u32 *iat;
      char *libname;

      if ( (imp->TimeDateStamp==0 ) && (imp->Name==0) )
        break;


      thunk = MakePtr(PIMAGE_THUNK_DATA32,
                      imp->Characteristics, base);
      iat= MakePtr(DWORD*,imp->FirstThunk, base);

      while ( 1 ) // Loop forever (or until we break out)
      {
        PIMAGE_IMPORT_BY_NAME ord;

        u32 addr;

        if ( thunk->u1.AddressOfData == 0 )
          break;

        if ( thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG )
        {
        //  printf("  %4u\n", thunk->u1.Ordinal & 0xFFFF);
            break;
        }
        else
        {
          PKERNEL_EXPORT exp;
          exp = kernel_export;

          ord = MakePtr(PIMAGE_IMPORT_BY_NAME,
                        thunk->u1.AddressOfData,base);
          *iat=-1;

          do
          {
            if(!strncmp(ord->Name,exp->name,16))
            {
              *iat = exp->address;
              break;
            }
            exp++;
          } while(exp->name != 0);
        };
        thunk++;            // Advance to next thunk
        iat++;
      }
      imp++;  // advance to next IMAGE_IMPORT_DESCRIPTOR
    }
  };

*/
