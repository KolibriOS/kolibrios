#include "kosnet/network.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    load_network_obj();

    char *host = "kolibrios.org";
    int port = 80;
    printf("Connecting to %s on port %d\n", host, port);

    struct addrinfo *addr_info;
    char port_str[16]; sprintf(port_str, "%d", port);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6 doesnt matter
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    if (getaddrinfo(host, port_str, 0, &addr_info) != 0) {
        printf("Host %s not found!\n", host);
        freeaddrinfo(addr_info);
        exit(-1);
    }
    printf("IP address of %s is %s\n", host, inet_ntoa(addr_info->ai_addr->sin_addr));  
    //printf("Host port = %d\n", addr_info->ai_addr->sin_port >> 8);

    char request[256];
    sprintf(request, "GET /en/ HTTP/1.1\r\nHost: %s\r\n\r\n", host);
    printf("request = %s\n", request);

    int sock = socket(AF_INET4, SOCK_STREAM, IPPROTO_TCP);

    puts("Connecting...\n");
    if (connect(sock, addr_info->ai_addr, addr_info->ai_addrlen) != 0) {
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

    freeaddrinfo(addr_info);

    closesocket(sock);
    puts("\n goodbye)\n");
	return 0;
}