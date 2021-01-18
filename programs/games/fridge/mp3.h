// Was written by maxcodehack

#pragma pack(push,1)
typedef struct 
{
unsigned	p00;
unsigned 	p04;
char 		*p08;
unsigned	p12;
unsigned	p16;
char		p20;
char		*p21;
} kol_struct70;
#pragma pack(pop)

int kol_file_70(kol_struct70 *k)
{
	__asm__ __volatile__ ("int $0x40"::"a"(70), "b"(k) : "memory");
}

int RunApp(char *app, char *param)
{
	kol_struct70 r;
	r.p00 = 7;
	r.p04 = 0;
	r.p08 = param;
	r.p12 = 0;
	r.p16 = 0;
	r.p20 = 0;
	r.p21 = app;
	return kol_file_70(&r);
}


void PlayMusic(char name[]) {
	char param[256] = "-h ";
	strcat(param, name);
	RunApp("/sys/media/ac97snd", param);
};
