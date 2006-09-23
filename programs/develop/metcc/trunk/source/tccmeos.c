/*
 *  TCCMEOS.C - KolibriOS/MenuetOS file output for the TinyC Compiler
 * 
 *  Copyright (c) 2006 Andrey Khalyavin
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

typedef struct {
	char magic[8];
	int version;
	int entry_point;
	int image_size;
	int memory_size;
	int stack;
	int params;
	int argv;
} IMAGE_MEOS_FILE_HEADER,*PIMAGE_MEOS_FILE_HEADER;
typedef struct _meos_section_info{
	int sh_addr;
	void* data;
	int data_size;
	int sec_num;
	struct _meos_section_info* next;
} meos_section_info;
typedef struct {
	TCCState* s1;
	IMAGE_MEOS_FILE_HEADER header;
	meos_section_info* code_sections;
	meos_section_info* data_sections;
	meos_section_info* bss_sections;
} me_info;

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
	Elf32_Rel *rel, *rel_, *rel_end;
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
			if (type != R_386_PC32 && type != R_386_32)
				continue;
			int sym = ELF32_R_SYM(rel->r_info);
			if (sym>symtab_section->data_offset/sizeof(Elf32_Sym))
				continue;
			Elf32_Sym* esym = ((Elf32_Sym *)symtab_section->data)+sym;
			int sect=esym->st_shndx;
			ss=findsection(me,sect);
			if (ss==0)
				ss=me->bss_sections;
			if (rel->r_offset>s->data_size)
				continue;
			if (type==R_386_PC32)
				*(int*)(rel->r_offset+s->data)=ss->sh_addr+esym->st_value-rel->r_offset-s->sh_addr-4;
			else if (type==R_386_32)
				*(int*)(rel->r_offset+s->data)+=ss->sh_addr+esym->st_value;
		}
        rel=rel_;
		s=s->next;
		if (s==0)
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
			si->data_size=s->data_offset;
			si->next=me->bss_sections;
			si->sec_num=i;
			me->bss_sections=si;
			continue;
		}
	}
	int addr;
	addr=sizeof(IMAGE_MEOS_FILE_HEADER);
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
	for (si=me->bss_sections;si;si=si->next)
	{
		si->sh_addr=addr;
		addr+=si->data_size;
	}
	addr+=4096;
	addr=(addr+4)&(~3);
	me->header.stack=addr;
	me->header.memory_size=addr;
	build_reloc(me);
}
int tcc_find_symbol_me(me_info* me, const char *sym_name)
{
	int i;
	int symtab;
	int strtab;
	symtab=0;
	strtab=0;
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
		return 0;
	Elf32_Sym* s,*se;
	char* name;
	s=(Elf32_Sym*)me->s1->sections[symtab]->data;
	se=(Elf32_Sym*)(((void*)s)+me->s1->sections[symtab]->data_offset);
	name=(char*)me->s1->sections[strtab]->data;
	while (s<se)
	{
		if (strcmp(name+s->st_name,sym_name)==0)
		{
			return s->st_value+findsection(me,s->st_shndx)->sh_addr;
		}
		s++;
	}
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
	relocate_common_syms();
	assign_addresses(&me);
	me.header.version=1;
	me.header.entry_point=tcc_find_symbol_me(&me,"start");
	me.header.params= tcc_find_symbol_me(&me,"__argv"); // <--
	me.header.argv= tcc_find_symbol_me(&me,"__path"); // <--
	
	f=fopen(filename,"wb");
    for (i=0;i<8;i++)
        me.header.magic[i]=me_magic[i];
	/*me.header.magic[0]='M';me.header.magic[1]='E';
	me.header.magic[2]='N';me.header.magic[3]='U';
	me.header.magic[4]='E';me.header.magic[5]='T';
	me.header.magic[6]='0';me.header.magic[7]='1';*/
	fwrite(&me.header,1,sizeof(IMAGE_MEOS_FILE_HEADER),f);
	meos_section_info* si;
	for(si=me.code_sections;si;si=si->next)
		fwrite(si->data,1,si->data_size,f);
	for (si=me.data_sections;si;si=si->next)
		fwrite(si->data,1,si->data_size,f);
	fclose(f);
	return 0;
}
