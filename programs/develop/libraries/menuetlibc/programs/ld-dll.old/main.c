#include"loader.h"

extern char __menuet__app_param_area[];

static char program_name[1024];
static char * argp;

static void extract_base_program_name(void)
{
 char * p;
 int i;
 p=strchr(__menuet__app_param_area,' ');
 if(!p)
 {
  i=strlen(__menuet__app_param_area);
 } else {
  i=((long)p)-((long)__menuet__app_param_area);
  if(!i)
  {
   __libclog_printf("No program name supplied\n");
   exit(-1);
  }
 }
 memcpy(program_name,__menuet__app_param_area,i);
 program_name[i]='\0';
 argp=&program_name[i+1];
}

dll_t * main_program,*dll;
char ** dll_load_table;

static char tmp[1024];

dll_t * try_load_dll(char * dllname)
{
 if(dllname[0]=='/') return load_dll(dllname);
 sprintf(tmp,"/RD/1/%s",dllname);
 if(!(dll=load_dll(tmp))) return;
 sprintf(tmp,"/HD/1/MENUETOS/%s",dllname);
 if(!(dll=load_dll(tmp))) return;
 sprintf(tmp,"/HD/1/MENUETOS/DLL/%s",dllname);
 if(!(dll=load_dll(tmp))) return;
 return load_dll(dllname);
}

int (* xmain)(void);

void main(void)
{
 __libclog_printf("Supplied parameters:\n");
 __libclog_printf("|%s|\n",__menuet__app_param_area);
 extract_base_program_name();
 init_dll();
 main_program=load_dll(program_name);
 if(!main_program)
 {
  __libclog_printf("Unable to open main program\n");
  exit(-1);
 }
 dll_load_table=(char **)mcoff_get_ref(main_program->obj,"__required_dll");
 if(dll_load_table)
 {
  int i;
  for(i=0;dll_load_table[i];i++)
  {
   if(!(dll=try_load_dll(dll_load_table[i])))
   {
    __libclog_printf("Unable to load dll '%s'\n",dll_load_table[i]);
    exit(-1);
   }
   xmain=(void *)mcoff_get_ref(dll->obj,"_DllMain");
   if(xmain) xmain();
  }
 }
 if(relocate_dlls()!=0)
 {
  __libclog_printf("Unable to relocate dynamic objects\n");
  exit(-1);
 }
 xmain=(void *)mcoff_get_ref(main_program->obj,"_app_main");
 if(!xmain)
 {
  __libclog_printf("Unable to find _app_main symbol in main program");
  exit(-1);
 }
 exit(xmain());
}
