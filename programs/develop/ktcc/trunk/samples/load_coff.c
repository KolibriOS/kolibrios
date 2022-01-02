#include <dlfcn.h>

/*Using the "coff" library in ktcc using "inputbox.obj" as an example*/

unsigned (*InputBox)(void* Buffer, char* Caption, char* Prompt, char* Default, unsigned long Flags, unsigned long BufferSize, void* RedrawProc);

void *InputBoxLib;

void load_coff()
{
  InputBoxLib = dlopen("/sys/lib/inputbox.obj", RTLD_GLOBAL);
  InputBox = dlsym(InputBoxLib,"InputBox");
}

int main()
{
    load_coff();
    char buffer[256];
    InputBox(buffer, "Hay!", "How do you do?", "Hmm?", 10, 256, 0);
    dlclose(InputBoxLib);
}
