/*
 *  TCCMEOS.C - KolibriOS/MenuetOS file output for the TinyC Compiler
 *
 *  Copyright (c) 2006 Andrey Khalyavin
 *  Copyright (c) 2021-2022 Coldy (KX extension)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "tcc.h"

typedef struct {
	char magic[8];
	int version;
	int entry_point;
	int image_size;
	int memory_size;
	int stack;
	int argv;
	int path;
} IMAGE_MEOS_FILE_HEADER,*PIMAGE_MEOS_FILE_HEADER;
typedef struct _meos_section_info{
	int sh_addr;
	void* data;
	int data_size;
	int sec_num;
	struct _meos_section_info* next;
} meos_section_info;
#ifdef TCC_TARGET_KX
typedef struct {
	void* data;
	int	  data_size;
} kx_import_table;
#endif
typedef struct {
	TCCState* s1;
	IMAGE_MEOS_FILE_HEADER header;
	meos_section_info* code_sections;
	meos_section_info* data_sections;
#ifdef TCC_TARGET_KX
	kx_import_table* imp_table;
#endif
	meos_section_info* bss_sections;
} me_info;

#ifdef TCC_TARGET_KX
	void kx_init(me_info* me);
	void kx_build_imports(me_info* me);
  	void kx_check_import_error(int reloc_type, const char* code_base, Elf32_Addr	offset, const char* symbol_name);
	long kx_get_header_length(me_info* me);
	void kx_write_header(me_info* me, FILE* f);
	void kx_write_imports(me_info* me, FILE* f);
	void kx_free(me_info* me);
#endif

int tcc_output_dbgme(const char *filename, me_info* me);


meos_section_info* findsection(me_info* me,int num)
{
	meos_section_info* si;
	for(si=me->code_sections;si;si=si->next)
	{
		if (si->sec_num==num)
			return si;
	}
	for (si=me->data_sections;si;si=si->next)
	{
		if (si->sec_num==num)
			return si;
	}
	for (si=me->bss_sections;si;si=si->next)
	{
		if (si->sec_num==num)
			return si;
	}
	return (meos_section_info*)0;
}

void build_reloc(me_info* me)
{
	int flag;
	Elf32_Rel *rel = 0, *rel_ = 0, *rel_end = 0;
	Section *sr;
	meos_section_info* s;
	meos_section_info* ss;
	s=me->code_sections;
	rel=0;
	rel_end=0;
	flag=0;
	for(;;)
	{
		sr=me->s1->sections[s->sec_num]->reloc;
		if (sr)
		{
			rel = (Elf32_Rel *) sr->data;
			rel_end = (Elf32_Rel *) (sr->data + sr->data_offset);
		}
		rel_=rel;
		while (rel_<rel_end){
			rel=rel_;
			int type = ELF32_R_TYPE(rel->r_info);
			rel_=rel+1;
			if (type != R_386_PC32 && type != R_386_32) {
				// gcc (and friends) object files is used?
				tcc_error("unsupported relocation type %d", type);
				continue;
			}
			int sym = ELF32_R_SYM(rel->r_info);
			if (sym>symtab_section->data_offset/sizeof(Elf32_Sym))
				continue;
			Elf32_Sym* esym = ((Elf32_Sym *)symtab_section->data)+sym;
			int sect=esym->st_shndx;
      int sh_addr;
			ss=findsection(me,sect);
      const char* sym_name = strtab_section->data + esym->st_name;
			int sym_index;
			Elf32_Sym* dyn_sym;
			// Import has more less priority in relation to local symbols
			if (ss==0)
			{
#ifdef TCC_TARGET_KX
				 sym_index = find_elf_sym(me->s1->dynsymtab_section, sym_name);
				if (sym_index == 0) {
#endif
          tcc_error_noabort("undefined import symbol '%s'", sym_name);
				  continue;
#ifdef TCC_TARGET_KX
			}
				dyn_sym = &((ElfW(Sym) *)me->s1->dynsymtab_section->data)[sym_index];
				sh_addr = dyn_sym->st_value;
				if (sh_addr == 0) {
					tcc_error_noabort("import symbol '%s' has zero value", sym_name);
					continue;
				}
        
        // Stop linking if incorrect import
				kx_check_import_error(type, s->data, rel->r_offset, sym_name);
#endif
			}
			else {
      if (esym->st_shndx == SHN_UNDEF)
				tcc_error("unresolved external symbol '%s'", sym_name);
				sh_addr = ss->sh_addr;
      }
			if (rel->r_offset>s->data_size)
				continue;
			if (type==R_386_PC32)
				*(int*)(rel->r_offset+s->data)+= sh_addr+esym->st_value-rel->r_offset-s->sh_addr;
			else if (type==R_386_32)
				*(int*)(rel->r_offset+s->data)+= sh_addr+esym->st_value;
		}
        rel=rel_;
		s=s->next;
		if (s==0) // what about multiple BSS sections ?
		{
			if (flag) break;
			s=me->data_sections;
			if (s==0) break;
			flag=1;
			continue;
		}
	}
}

void assign_addresses(me_info* me)
{
	int i;
	meos_section_info* si;
	for (i=1;i<me->s1->nb_sections;i++)
	{
		Section* s=me->s1->sections[i];
		if (strcmp(".text",s->name)==0)
		{
			si=tcc_malloc(sizeof(meos_section_info));
			si->data=s->data;
			si->data_size=s->data_offset;
			si->next=me->code_sections;
			si->sec_num=i;
			me->code_sections=si;
			continue;
		}
		if (strcmp(".data",s->name)==0)
		{
			si=tcc_malloc(sizeof(meos_section_info));
			si->data=s->data;
			si->data_size=s->data_offset;
			si->next=me->data_sections;
			si->sec_num=i;
			me->data_sections=si;
			continue;
		}
		if (strcmp(".bss",s->name)==0)
		{
			si=tcc_malloc(sizeof(meos_section_info));
			si->data=s->data;
			si->data_size=s->data_offset;
			si->next=me->bss_sections;
			si->sec_num=i;
			me->bss_sections=si;
			continue;
		}
	}
	int addr;
	addr=sizeof(IMAGE_MEOS_FILE_HEADER);
#ifdef TCC_TARGET_KX 
	addr += kx_get_header_length(me);
#endif
	for (si=me->code_sections;si;si=si->next)
	{
		si->sh_addr=addr;
		addr+=si->data_size;
	}
	for (si=me->data_sections;si;si=si->next)
	{
		si->sh_addr=addr;
		addr+=si->data_size;
	}
	me->header.image_size=addr;
#ifdef TCC_TARGET_KX 
	kx_build_imports(me);
	addr = me->header.image_size;
#endif
	for (si=me->bss_sections;si;si=si->next)
	{
		si->sh_addr=addr;
		addr+=si->data_size;
	}
	if (me->s1->pe_stack_size < 4096)
        addr+=4096;
    else
        addr += me->s1->pe_stack_size;
	addr=(addr+4)&(~3);
	me->header.stack=addr;
	me->header.memory_size=addr;
	build_reloc(me);
}


const char *tcc_get_symbol_name(int st_name)
// return string by index from stringtable section
{
	const char *sym_name = strtab_section->data + st_name;
	return sym_name;
}

int tcc_find_symbol_me(me_info* me, const char *sym_name, int* addr)
{
	int i, symtab = 0, strtab = 0;
	*addr = 0;
	for (i=1;i<me->s1->nb_sections;i++)
	{
		Section* s;
		s=me->s1->sections[i];
		if (strcmp(s->name,".symtab")==0)
		{
			symtab=i;
		}
		if (strcmp(s->name,".strtab")==0)
		{
			strtab=i;
		}
	}
	if (symtab==0 || strtab==0)
	{
        tcc_error_noabort("undefined sections .symtab or .strtab on linking '%s'", sym_name);
		return 0;
	}
	Elf32_Sym* s,*se;
	char* name;
	s=(Elf32_Sym*)me->s1->sections[symtab]->data;
	se=(Elf32_Sym*)(((void*)s)+me->s1->sections[symtab]->data_offset);
	name=(char*)me->s1->sections[strtab]->data;
	while (s<se)
	{
		if (strcmp(name+s->st_name,sym_name)==0)
		{
			*addr = s->st_value+findsection(me,s->st_shndx)->sh_addr;
			return 1;
		}
		s++;
	}
    tcc_error_noabort("undefined symbol '%s'", sym_name);
	return 0;
}

const char* me_magic="MENUET01";
int tcc_output_me(TCCState* s1,const char *filename)
{
	me_info me;
    int i;
    FILE* f;
    //printf("%d\n",s1->nb_sections);
	memset(&me,0,sizeof(me));
	me.s1=s1;
	tcc_add_runtime(s1);
#ifdef TCC_TARGET_KX
	kx_init(&me);
#endif
	relocate_common_syms();
	assign_addresses(&me);
    
	if (s1->do_debug)
		tcc_output_dbgme(filename, &me);

	if (!tcc_find_symbol_me(&me, "start",  &me.header.entry_point) |
	    !tcc_find_symbol_me(&me, "__argv", &me.header.argv) |
	    !tcc_find_symbol_me(&me, "__path", &me.header.path)) {
	    exit(1);
	}

	if((f=fopen(filename,"wb"))==NULL){
		tcc_error("could not create '%s': %s", filename, strerror(errno));
	}

    for (i=0;i<8;i++)
        me.header.magic[i]=me_magic[i];
	fwrite(&me.header,1,sizeof(IMAGE_MEOS_FILE_HEADER),f);
#ifdef TCC_TARGET_KX
	kx_write_header(&me, f);
#endif
	meos_section_info* si;
	for(si=me.code_sections;si;si=si->next)
		fwrite(si->data,1,si->data_size,f);
	for (si=me.data_sections;si;si=si->next)
		fwrite(si->data,1,si->data_size,f);
#ifdef TCC_TARGET_KX	
	kx_write_imports(&me, f);
	kx_free(&me);
#else
	if (!s1->nobss)
	{
		for (si=me.bss_sections;si;si=si->next)
		{
	    	if (si->data == NULL)
			{
	//         	printf("\nError! BSS data is NULL! size:%i",(int)si->data_size);
	         	si->data = calloc(si->data_size, 1);
	      	}
			fwrite(si->data, 1, si->data_size, f);
		}
	}
/*
    if (me.bss_sections) // Siemargl testin, what we lose
    {
        tcc_error_noabort("We lose .BSS section when linking KOS32 executable");
    }
*/
#endif
	fclose(f);
	return 0;
}

#if !defined(_WIN32) && !defined(TCC_TARGET_MEOS_LINUX)

static inline int get_current_folder(char* buf, int bufsize){
    register int val;
    asm volatile ("int $0x40":"=a"(val):"a"(30), "b"(2), "c"(buf), "d"(bufsize));
    return val;
}


char *getcwd(char *buf, size_t size)
{
	int rc = get_current_folder(buf, size);
	if (rc > size)
	{
		errno = ERANGE;
		return 0;
	}
	else
		return buf;
}

#endif


static FILE *src_file;
static int next_src_line;

void close_source_file()
{
    if (src_file)
        fclose(src_file);
    src_file = NULL;
}

void load_source_file(char *fname)
{
    close_source_file();
    src_file = fopen(fname,"rt");
	if (!src_file) return;
	next_src_line = 1;
}

int get_src_lines(char *buf, int sz, int start, int end)
// 1 if read
{
    char line[255], *ch;
    strcpy(buf, "");
	if (!src_file) return 0;
    while (next_src_line < start) // skip
    {
        ch = fgets(line, sizeof line, src_file);
        if (!ch) return 0;
        next_src_line++;
    }
    while (next_src_line <= end)
    {
        ch = fgets(line, sizeof line, src_file);
        if (!ch) return 0;
        next_src_line++;
        strncat(buf, line, sz - strlen(buf) - 1);
    }
    // remove newlines
    for (ch = buf; *ch; ch++)
        if (strchr("\t\n\r", *ch)) *ch = ' ';

    return 1;
}

int tcc_output_dbgme(const char *filename, me_info* me)
// by Siemargl. Writes filename.dbg file for source code level debuggin with MTDBG
// return 1 on error
{
	FILE 	*fdbg;
	char	fname[400],
            buf[80]; // no more fits in mtdbg string

	strcpy(fname, filename);
	strcat(fname, ".dbg");
	fdbg = fopen(fname,"wt");
	if (!fdbg) return 1;

	meos_section_info *si, *ss;
    fputs(".text\n", fdbg); // just for mtbg

    // print symbol table with resolved addresses
    Elf32_Sym* esym;
    for (esym = (Elf32_Sym*)symtab_section->data; esym <= (Elf32_Sym*)(symtab_section->data + symtab_section->data_offset); esym++)
    {
        if (esym->st_info == 0 || esym->st_info == 4) continue;
        int sect = esym->st_shndx;
        ss = findsection(me, sect);
        const char *sym_name = strtab_section->data + esym->st_name;
        if (ss == 0)
        {
            fprintf(fdbg, "undefined symbol '%s' type(%d)\n", sym_name, esym->st_info);
            continue;
        }
        fprintf(fdbg, "0x%X %s\n", ss->sh_addr + esym->st_value, sym_name); // removed type(%d)   esym->st_info
    }

    fputs(".text source code links\n", fdbg); // just for mtbg
    // print symbol table with resolved addresses
    Stab_Sym *stab;
    char    *str = "", *cur_file = "???", cur_fun[255];
    int cur_line = 0, cur_fun_addr = 0, fun_flag = 0;
    strcpy(cur_fun, "fn???");
    for (stab = (Stab_Sym*)stab_section->data; stab <= (Stab_Sym*)(stab_section->data + stab_section->data_offset); stab++)
    {
        str = "";
        switch(stab->n_type)
        {
        case 100:   // source file, or path
            if (stab->n_strx)
            {
                cur_file = stabstr_section->data + stab->n_strx;
                load_source_file(cur_file);
            }
            else
                cur_file = "???";
            strcpy(cur_fun, "fn???");
            cur_line = 0;
            cur_fun_addr = 0;
            fun_flag = 0;
            break;
        case 36:    // func
            cur_fun_addr = 0;
            if (stab->n_strx)
            {
                strcpy(cur_fun, stabstr_section->data + stab->n_strx);
                str = strchr(cur_fun, ':');
                if (str) *str = '\0';
                tcc_find_symbol_me(me, cur_fun, &cur_fun_addr);
                cur_line = stab->n_desc;
                fun_flag = 1;
                //fprintf(fdbg, "0x%X %s() line(%d)\n", cur_fun_addr, cur_fun, cur_line); // commented as conflicted with direct address
            }
            else
                strcpy(cur_fun, "fn???");
            break;
        case 68:    // N_SLINE
            if (stab->n_value == 0 ) continue;  // skip zero offset line
            if (fun_flag) // skip string {, as duplicates address
            {
                fun_flag = 0;
                continue;
            }

            int line;
            if (stab->n_desc > cur_line)
                line = cur_line + 1;
            else
                line = cur_line;
            //fprintf(fdbg, "0x%X LINES %d-%d \n", cur_fun_addr + stab->n_value, line,  stab->n_desc);
            if (get_src_lines(buf, sizeof buf, line, stab->n_desc))
                fprintf(fdbg, "0x%X %s\n", cur_fun_addr + stab->n_value, buf);

            cur_line = stab->n_desc;
            break;
        default:
            continue;
        }
/*
        if (stab->n_strx)
            str = stabstr_section->data + stab->n_strx;
        fprintf(fdbg, "0x%X type(%d) str(%s) desc(%d)\n", stab->n_value, stab->n_type, str, stab->n_desc);
*/
    }

/*        for(; si; si = si->next)
        {
            Section *sr;
            Elf32_Rel *rel, *rel_end;
            for(sr = me->s1->sections[si->sec_num]->reloc; sr; )
            {
                for (rel = (Elf32_Rel *) sr->data, rel_end = (Elf32_Rel *) (sr->data + sr->data_offset); rel < rel_end; rel++)
                {
                    int type = ELF32_R_TYPE(rel->r_info);
                    if (type != R_386_PC32 && type != R_386_32)
                        continue;
                    int sym = ELF32_R_SYM(rel->r_info);
                    if (sym > symtab_section->data_offset / sizeof(Elf32_Sym))
                        continue;
                    Elf32_Sym* esym = ((Elf32_Sym*)symtab_section->data) + sym;
                    int sect = esym->st_shndx;
                    ss = findsection(me, sect);
                    const char *sym_name = strtab_section->data + esym->st_name;
                    if (ss == 0)
                    {
                        fprintf(fdbg, "undefined symbol '%s'\n", sym_name);
                        continue;
                    }
                    if (rel->r_offset > si->data_size) continue;
                    fprintf(fdbg, "\t0x%X %s\n", ss->sh_addr + esym->st_value, sym_name);
                }
            }
        }
*/
    close_source_file();
	fclose(fdbg);
	return 0;
}
