#include"mcoff.h"
#include"string.h"
#include<stdlib.h>

SCNHDR * find_section(char * name,coffobj_t * obj)
{
 char newname[9];
 int i,j;
 if(!name || !obj) return NULL;
 memset(newname,0,9);
 memcpy(newname,name,8);
 j=strlen(newname);
 for(i=0;i<obj->co_filehdr->f_nscns;i++)
 {
  if(!strncmp(obj->co_sections[i].s_name,newname,j)) return &obj->co_sections[i];
 }
 return NULL;
}

int read_section_data(coffobj_t * obj,SCNHDR * hdr,void ** readp)
{
 *readp=malloc(hdr->s_size);
 if(!(*readp)) return -1;
 memcpy(*readp,obj->co_loadptr+hdr->s_scnptr,hdr->s_size);
 return 0;
}
