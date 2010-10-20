int cmd_kerver(char param[])
{
char		*kvbuf;
char		*vA, *vB, *vC, *vD;
unsigned	*Rev;

kvbuf = malloc(16);
kol_get_kernel_ver(kvbuf);
vA = kvbuf+0;
vA = *vA;
vB = kvbuf+1;
vB = *vB;
vC = kvbuf+2;
vC = *vC;
vD = kvbuf+3;
vD = *vD;
Rev = kvbuf+5;
Rev = *Rev;

#if LANG_ENG
	printf ("  KolibriOS v%d.%d.%d.%d. Kernel SVN-rev.: %d\n\r", vA, vB, vC, vD, Rev);
#elif LANG_RUS
	printf ("  КолибриОС v%d.%d.%d.%d. SVN-рев. ядра: %d\n\r", vA, vB, vC, vD, Rev);
#endif
free(kvbuf);
return TRUE;
}