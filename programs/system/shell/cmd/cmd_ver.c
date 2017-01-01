
int cmd_ver(char param[])
{

if (!strcmp(param, "kernel"))
	{
	char		*kvbuf;
	char		*vA, *vB, *vC, *vD;
	unsigned	*Rev;

	kvbuf = malloc(16);
	kol_get_kernel_ver(kvbuf);
	vA = kvbuf+0;
	vB = kvbuf+1;
	vC = kvbuf+2;
	vD = kvbuf+3;
	Rev = (unsigned*)(kvbuf + 5);

	#if LANG_ENG
		printf ("  KolibriOS v%d.%d.%d.%d. Kernel SVN-rev.: %d\n\r", *vA, *vB, *vC, *vD, *Rev);
	#elif LANG_RUS
		printf ("  KolibriOS v%d.%d.%d.%d. SVN-рев. ядра: %d\n\r", *vA, *vB, *vC, *vD, *Rev);
	#endif

	free(kvbuf);

        return TRUE;
	}

if (!strcmp(param, "cpu"))
   {
   unsigned a, b, c, d;
   char str[13];

   str[12] = '\0';

   asm ("cpuid" :
		"=a" (a),
        "=b" (b),
        "=c" (c),
        "=d" (d):
	"a"(0));

    str[0] = (b&0x000000ff)	>> 0;
    str[1] = (b&0x0000ff00)	>> 8;
    str[2] = (b&0x00ff0000)	>> 16;
    str[3] = (b&0xff000000)	>> 24;

    str[4] = (d&0x000000ff)	>> 0;
    str[5] = (d&0x0000ff00)	>> 8;
    str[6] = (d&0x00ff0000)	>> 16;
    str[7] = (d&0xff000000)	>> 24;

    str[8] = (c&0x000000ff)	>> 0;
    str[9] = (c&0x0000ff00)	>> 8;
    str[10] = (c&0x00ff0000)	>> 16;
    str[11] = (c&0xff000000)	>> 24;

    printf("%s\n\r", str);
    return TRUE;
   }


printf ("  Shell v%s\n\r", SHELL_VERSION);
return TRUE;
}

