#include"ld-dll.h"
#include"dll_desc.h"

dll_t * main_program;

static char * std_dll_paths[]={
 "/rd/1/",
 "/hd/1/menuetos/",
 "/hd/1/menuetos/dll/",
 "./",
 NULL
};

static char dll_name_buf[1024];

dll_t * do_load_dll(
void main(void)
{
 init_dll();
 main_program=load_dll("/rd/1/test.app");
 if(!main_program)
 {
  dprintf("Main load failed\n");
  exit(-1);
 }
 { dll_t * tmp;
 if(!(tmp=load_dll("/rd/1/vcrt.dll")))
 {
  dprintf("Unable to load vcrt.dll\n");
  exit(-1);
 }
 dprintf("Looking for entry point\n");
 tmp->entry_point=(void *)mcoff_get_ref(tmp->obj,"_DllMain");
 if(tmp->entry_point) tmp->entry_point();
 if(!(tmp=load_dll("MOSKRNL.SO")))
  if(!(tmp=load_dll("/RD/1/MOSKRNL.SO")))
   if(!(tmp=load_dll("/HD/1/MENUETOS/MOSKRNL.SO")))
   {
    dprintf("Unable to load moskrnl.so\n");
    exit(-1);
   }
 tmp->entry_point=(void *)mcoff_get_ref(tmp->obj,"_DllMain");
 if(tmp->entry_point) tmp->entry_point();
 }
 relocate_dlls();
 main_program->entry_point=(void *)mcoff_get_ref(main_program->obj,"_main");
 if(!main_program->entry_point)
 {
  dprintf("Failed to find main program entry point\n");
  exit(-1);
 }
 main_program->entry_point();
 exit(0);
}
