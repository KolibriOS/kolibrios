
int cmd_about(char param[])
{

char message[] = {
"\
\n\r\
Shell %s\n\r\n\r\
"};

printf(message, SHELL_VERSION); 
return TRUE;
}
