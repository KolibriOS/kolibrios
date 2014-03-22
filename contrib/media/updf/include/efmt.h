#ifndef __EFMT_H
#define __EFMT_H

#ifndef __PACKED__
#define __PACKED__	__attribute__((packed))
#endif

typedef struct
{
    unsigned long magic		__PACKED__;
    unsigned char bitness	__PACKED__;
    unsigned char endian	__PACKED__;
    unsigned char elf_ver_1	__PACKED__;
    unsigned char res[9]	__PACKED__;
    unsigned short file_type	__PACKED__;
    unsigned short machine	__PACKED__;
    unsigned long elf_ver_2	__PACKED__;
    unsigned long entry_pt	__PACKED__;
    unsigned long phtab_offset	__PACKED__;
    unsigned long shtab_offset	__PACKED__;
    unsigned long flags		__PACKED__;
    unsigned short file_hdr_size __PACKED__;
    unsigned short phtab_ent_size __PACKED__;
    unsigned short num_phtab_ents __PACKED__;
    unsigned short shtab_ent_size __PACKED__;
    unsigned short num_sects	__PACKED__;
    unsigned short shstrtab_index __PACKED__;
} elf_file_t;

typedef struct
{
    unsigned long sect_name	__PACKED__;
    unsigned long type		__PACKED__;
    unsigned long flags		__PACKED__;
    unsigned long virt_adr	__PACKED__;
    unsigned long offset	__PACKED__;
    unsigned long size		__PACKED__;
    unsigned long link		__PACKED__;
    unsigned long info		__PACKED__;
    unsigned long align		__PACKED__;
    unsigned long ent_size	__PACKED__;
} elf_sect_t;

typedef struct
{
    unsigned long adr		__PACKED__;
    unsigned char type		__PACKED__;
    unsigned long symtab_index:24 __PACKED__;
    unsigned long addend	__PACKED__;
} elf_reloc_t;

typedef struct
{
    unsigned long name		__PACKED__;
    unsigned long value		__PACKED__;
    unsigned long size		__PACKED__;
    unsigned type:4		__PACKED__;
    unsigned binding:4		__PACKED__;
    unsigned char zero		__PACKED__;
    unsigned short section	__PACKED__;
} elf_sym_t;

typedef struct
{
 unsigned char * file, * sects,*bss,*symtab;
 char * strtab;
 unsigned long bss_sect_num,entry;
} exe_file_t;

int get_elf_section_name(exe_file_t * f,unsigned short sect_num,char * namebuf);
int get_elf_section_addr(exe_file_t * f,unsigned short sect_num,unsigned long * adr);
int get_elf_symbol_addr(exe_file_t * f,unsigned long i,
    unsigned long * sym_val,unsigned short sect_num,
    int (* lookup_fn)(char * symname,unsigned long * val));
int do_elf_relocation(exe_file_t * f,elf_reloc_t * reloc,
 unsigned short sect_num,unsigned long symtab_sect_num,
 int (* sym_lookup_fn)(char * symname,unsigned long * val),int ignore_ext);
int get_elf_section_index(exe_file_t * f,char * sect_name,unsigned long * i);
int elf_load_from_mem(exe_file_t * f);
int relocate_elf_file(exe_file_t * f,
 int (* sym_lookup_fn)(char * symname,unsigned long * val),int ignore_ext);
int get_elf_symbol_value(exe_file_t * f,char * symname,unsigned long * symval);


#endif
