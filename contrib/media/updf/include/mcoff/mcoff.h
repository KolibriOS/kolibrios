#ifndef __MENUETOS_MCOFF_H
#define __MENUETOS_MCOFF_H

#include"_coff.h"

typedef struct {
 char			* co_loadptr;
 unsigned long		  co_loadaddr;
 unsigned long		  co_filesize;
 FILHDR			* co_filehdr;
 SCNHDR			* co_sections;
 SYMENT			* co_symtab;
 char			* co_strtab; 
 char			* co_bssptr;
 unsigned long		  co_bssaddr;
 unsigned long		  co_bsssize;
 unsigned long		  co_bsssectnum;
} coffobj_t;

coffobj_t * mcoff_load_file(char * fname);
void unload_coff_file(coffobj_t * obj);
SCNHDR * find_section(char * name,coffobj_t * obj);
int read_section_data(coffobj_t * obj,SCNHDR * hdr,void ** readp);
SYMENT * find_coff_symbol(coffobj_t * obj,char * objname);

typedef int (* symlookupfn_t)(coffobj_t *,unsigned long *,unsigned long *,int);
int relocate_coff_file(coffobj_t * obj,symlookupfn_t lookupfn);
int mcoff_std_symlookupfn(coffobj_t * obj,unsigned long * sym_val,
 unsigned long * sym_sect,int index);
unsigned long mcoff_get_ref(coffobj_t * obj,char * symname);

/*
Your lookup function can be similar to this:

int mcoff_std_symlookupfn(coffobj_t * obj,unsigned long * sym_val,
 unsigned long * sym_sect,int index)
{
 SYMENT * symtab,* lookup;
 char xname[9];
 char * symnamep;
 symtab=obj->co_symtab+index;
 *sym_sect=(unsigned long)symtab->e_scnum;
 if(symtab->e_scnum>0)
 {
  *sym_val=symtab->e_value;
  return 0;
 }
 if(symtab->e.e.e_zeroes==0)
 {
  symnamep=(char *)(((long)obj->co_strtab)+symtab->e.e.e_offset);
 } else {
  symnamep=(char *)symtab->e.e_name;
  memset(xname,0,9);
  memcpy(xname,symnamep,8);
  symnamep=xname;
 }
 lookup=find_coff_symbol(obj,symnamep);
 if(!lookup) return -1;
 *sym_val=lookup->e_value+obj->co_sections[lookup->e_scnum-1].s_scnptr+obj->co_loadaddr;
 return 0;
}
*/

#endif
