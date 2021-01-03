
struct sysdir
{
   char name[64];
   char path[64];
} sysdir;


:int SetAdditionalSystemDirectory(dword tName, tPath)
{
	int i;
	strcpy(#sysdir.name, tName);
	strcpy(#sysdir.path, tPath);
	$mov eax, 30
	$mov ebx, 3
	ECX = #sysdir;
	$int 0x40
}
