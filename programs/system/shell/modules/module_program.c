
int program_run(char cmd[], char param[])
{

kol_struct70	k70;

k70.p00 = 7;
k70.p04 = 0;
k70.p08 = param;
k70.p12 = 0;
k70.p16 = 0;
k70.p20 = 0;
k70.p21 = cmd;

return kol_file_70(&k70);
}
