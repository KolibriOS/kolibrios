
int cmd_exit(char param[])
{
free(ALIASES);
_exit(1);
kol_exit();
return TRUE;
}

