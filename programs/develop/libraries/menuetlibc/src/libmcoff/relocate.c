#include"mcoff.h"
#include<stdlib.h>
#include"string.h"

static int do_coff_relocation(coffobj_t * obj,unsigned long relno,
 SCNHDR * sect,symlookupfn_t lookup_fn)
{
 int r_delta,t_delta=0;
 unsigned long sym_val,sym_sect;
 unsigned long * where;
 int err;
 RELOC * rel;
 r_delta=obj->co_loadaddr+sect->s_scnptr-sect->s_vaddr;
 rel=((RELOC *)(obj->co_loadaddr+sect->s_relptr))+relno;
 where=(unsigned long *)(r_delta+rel->r_vaddr);
 err=lookup_fn(obj,&sym_val,&sym_sect,rel->r_symndx);
 if(err!=0)
 {
  printf("Unable to find symbol relno=%u\n",relno);
  return err;
 }
 if(sym_sect!=0)
 {
  sect=&obj->co_sections[sym_sect-1];
  t_delta=obj->co_loadaddr+sect->s_scnptr-sect->s_vaddr;
 }
 switch(rel->r_type)
 {
  case RELOC_ADDR32:
   if(!sym_sect)
    *where=sym_val;
   else
    *where+=t_delta;
   break;
  case RELOC_REL32:
   if(!sym_sect)
    *where+=sym_val-r_delta;
   else
    *where+=t_delta-r_delta;
   break;
  default:
   printf("Invalid relocation type\n");
   return -1;
 }
 return 0;
}

int relocate_coff_file(coffobj_t * obj,symlookupfn_t lookupfn)
{
 int s,r;
 for(s=0;s<obj->co_filehdr->f_nscns;s++)
 {
  for(r=0;r<obj->co_sections[s].s_nreloc;r++)
  {
   if(do_coff_relocation(
    obj,
    r,
    &obj->co_sections[s],
    lookupfn)!=0) return -1;
  }
 }
 return 0;
}

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
