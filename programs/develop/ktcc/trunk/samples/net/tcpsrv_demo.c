#include <net/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char msg1[]="Hello!"; 
char msg2='\0';

int main()
{
    struct sockaddr addr={AF_INET4, PORT(23) , 0, 0};
    
    int sk1=socket(AF_INET4, SOCK_STREAM, IPPROTO_TCP);
    printf("Open socket: %d. Status: %s\n",sk1, strerror(errno));
    
    bind(sk1, &addr,sizeof(addr));
    printf("Socket binding. Status: %s\n", strerror(errno));
    
    listen(sk1, 1);
    printf("Listening to a socket. Status: %s\n", strerror(errno));
    printf("You can connect to 'tcp server' via 'telnet' on localhost:23 !");
    
    int sk2 = accept(sk1, &addr, sizeof(addr));
    printf("Accept done. Status: %s\n", strerror(errno));
    
    send(sk2, msg1, strlen(msg1),MSG_NOFLAG);
    printf("Send message: '%s'. Status: %s\n",msg1, strerror(errno));
    puts("Received data:");
    while(msg2!='!')
    {
        recv(sk2, &msg2, 1, MSG_NOFLAG);
        printf("%c",msg2);
    }
    close(sk1);
    close(sk2);
    puts("\nGood bye!");
    exit(0);
}
