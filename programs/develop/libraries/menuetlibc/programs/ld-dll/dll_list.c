#include"ld-dll.h"

#define MAX_DLL		32

static dll_t dll_list[MAX_DLL];

SYMENT * dl_find_dll_symbol(char * name,dll_t ** xdll)
{
 dll_t * dll;
 SYMENT * __ret;
 int i;
 for(dll=dll_list+0,i=0;i<MAX_DLL;i++,dll++)
 {
  if(dll->obj)
  {
   __ret=find_coff_symbol(dll->obj,name);
   if(__ret) 
   {
    *xdll=dll;
    return __ret;
   }
  }
 }
 *xdll=(dll_t *)NULL;
 return 0;
}

unsigned long dl_get_ref(char * symname)
{
 dll_t * dll;
 SYMENT * sym=dl_find_dll_symbol(symname,&dll);
 if(!sym && !dll) return 0;
 return sym->e_value+dll->obj->co_sections[sym->e_scnum-1].s_scnptr+dll->obj->co_loadaddr;
}

void init_dll(void)
{
 int i;
 for(i=0;i<MAX_DLL;i++)
  dll_list[i].obj=NULL;
}

dll_t * load_dll(char * name)
{
 dll_t * p;
 int i;
 dprintf("Load dll '%s'\n",name);
 for(i=0,p=dll_list+0;i<MAX_DLL;i++,p++)
 {
  if(!p->obj)
  {
   p->obj=mcoff_load_file(name);
   p->d_name=strdup(name);
   if(!p->obj) return NULL;
   return p;
  }
 }
 return NULL;
}

dll_t * find_dll(char * name)
{
 dll_t * p;
 int i,j=strlen(name);
 for(i=0,p=dll_list+0;i<MAX_DLL;i++,p++)
 {
  if(p->obj)
  {
   if(strlen(p->d_name)==j &&
      !strncmp(name,p->d_name,j)) return p;
  }
 }
 return NULL;
}

int dll_symlookupfn(coffobj_t * obj,unsigned long * sym_val,
 unsigned long * sym_sect,int index)
{
 SYMENT * symtab;
 unsigned long lookup;
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
 lookup=kexport_lookup(symnamep);
 if(lookup)
 {
  *sym_val=lookup;
  return 0;
 }
 lookup=dl_get_ref(symnamep);
 if(!lookup) return -1;
 *sym_val=lookup;
 return 0;
}

int relocate_dlls(void)
{
 int i;
 dll_t * dll;
 for(i=0,dll=dll_list+0;i<MAX_DLL;i++,dll++)
  if(dll->obj)
   if(relocate_coff_file(dll->obj,dll_symlookupfn)) return -1;
 return 0;
}
