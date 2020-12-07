#include <net/network.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>

struct addrinfo *res;
char host[256];

void main()
{
    con_init_console_dll(); 
    networklib_init(); 
    con_set_title("nslookup demo");
    printf("Host name to resolve: ");
    con_gets(host, 256);
    host[strlen(host)-1] = '\0'; 
    if(getaddrinfo(host ,0, 0, &res)!=0)
    {
        puts("Host not found!");
        freeaddrinfo(res);
        con_exit(0);
    }
    else
    {
        printf("%s",inet_ntoa(res->ai_addr->sin_addr));  
        freeaddrinfo(res);
        con_exit(0);  
    }
}
