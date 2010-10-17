
int cmd_about(char param[])
{

char message[] = {
"\
\n\r\
Shell for KolibriOS\n\r\
version %s\n\r\n\r\
  author: Oleksandr Bogomaz aka Albom\n\r\
  e-mail: albom85@yandex.ru\n\r\
    site: http://albom85.narod.ru/\n\r\n\r\
"};

printf(message, SHELL_VERSION); 
return TRUE;
}
