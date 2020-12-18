#include <net/network.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
	con_init_console_dll(); 
    networklib_init();
    con_set_title("http request demo using raw sockets");

    char *host = "kolibrios.org";
    int port = 80;
    printf("Connecting to %s on port %d\n", host, port);

    struct addrinfo *addr_info;
    if (getaddrinfo(host, 0, 0, &addr_info) != 0) {
        printf("Host %s not found!\n", host);
        freeaddrinfo(addr_info);
        exit(-1);
    }
    unsigned int host_sin_addr = addr_info->ai_addr->sin_addr;
    printf("IP address of %s is %s\n", host, inet_ntoa(host_sin_addr));  
    //printf("Host port = %d\n", addr_info->ai_addr->sin_addr >> 8);
    freeaddrinfo(addr_info);

    char request[256];
    sprintf(request, "GET /en/ HTTP/1.1\r\nHost: %s\r\n\r\n", host);
    printf("request = %s\n", request);

    int sock = socket(AF_INET4, SOCK_STREAM, IPPROTO_TCP);
    sockaddr sock_addr;
    sock_addr.sin_family = AF_INET4;
    sock_addr.sin_port = 80 << 8; // dirty hack, normally need to use htons(80) but there is no such here in libraries
    sock_addr.sin_addr = host_sin_addr;
    sock_addr.sin_zero = 0;

    puts("Connecting...\n");
    if (connect(sock, &sock_addr, sizeof(sock_addr)) != 0) {
    	printf("Connection failed, err_code = %d\n", err_code);
    	exit(err_code);
    }
    puts("Connected successfully\n");

    puts("Sending request...\n");
    if (send(sock, request, strlen(request), MSG_NOFLAG) == -1) {
    	printf("Sending failed, err_code = %d\n", err_code);
    	exit(err_code);
    }
    puts("Request sended successfully, waiting for response...\n");

    char buf[512 + 1];
    if (recv(sock, buf, 512, MSG_NOFLAG) == -1) {
    	printf("Receive failed, err_code = %d\n", err_code);
    	exit(err_code);
    }

    printf("Response = %s\n", buf);

    con_exit(0);
	return 0;
}