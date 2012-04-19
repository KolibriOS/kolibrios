
int cmd_pwd(char param[])
{
printf ("  %s%c\n\r", cur_dir, cur_dir[strlen(cur_dir)-1]=='/'?' ':'/' );
return TRUE;
}
