
int cmd_about(char param[])
{

char message[] = {"Shell %s\n\r"};

printf(message, SHELL_VERSION); 
return TRUE;
}

