#include"mcoff.h"
#include<stdlib.h>
#include"string.h"

/* static inline void nprintf(char * s,int n)
{
 printf("nprintf(%u, '",n);
 for(;n;n--)
  putch(*s++);
 printf("')");
} */

SYMENT * find_coff_symbol(coffobj_t * obj,char * objname)
{
 int namelen;
 int symno;
 int xlen;
 SYMENT * symtab;
 char * symnamep,symnamelen;
 symtab=obj->co_symtab;
 namelen=strlen(objname);
 for(symno=0;symno<obj->co_filehdr->f_nsyms;symno++,symtab++)
 {
  if(symtab->e.e.e_zeroes==0)
  {
   symnamep=(char *)(((long)obj->co_strtab)+symtab->e.e.e_offset);
   symnamelen=strlen(symnamep);
/*   printf("strtab=%u %x %u ",symtab->e.e.e_offset,symnamep,symnamelen); */
  } else {
   symnamep=(char *)symtab->e.e_name;
   symnamelen=strlen(symnamep);
   if(symnamelen>E_SYMNMLEN)
    symnamelen=E_SYMNMLEN;
  }
/*  nprintf(symnamep,symnamelen);
  printf("\n"); */
  if(symtab->e_scnum!=0 && namelen==symnamelen && !strncmp(objname,symnamep,namelen))
   return symtab;
 }
 return NULL;
}

unsigned long mcoff_get_ref(coffobj_t * obj,char * symname)
{
 SYMENT * sym=find_coff_symbol(obj,symname);
 if(!sym) return 0;
 return sym->e_value+obj->co_sections[sym->e_scnum-1].s_scnptr+obj->co_loadaddr;
}
