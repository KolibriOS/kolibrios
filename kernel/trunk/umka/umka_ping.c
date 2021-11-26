/*
    umka_shell: User-Mode KolibriOS developer tools, the ping
    Copyright (C) 2020  Ivan Baravy <dunkaist@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "shell.h"
#include "umka.h"
#include "vnet.h"

uint8_t packet[4096] = {0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 'a','b',
                        'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
                        'y', 'z', '0', '1', '2', '3', '4', '5'};

static int
tap_alloc(char *dev) {
    int flags = IFF_TAP | IFF_NO_PI;
    struct ifreq ifr;
    int fd, err;
    char *clonedev = "/dev/net/tun";

    if( (fd = open(clonedev , O_RDWR | O_NONBLOCK)) < 0 )
    {
        perror("Opening /dev/net/tun");
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = flags;

    if(*dev)
    {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 )
    {
        perror("ioctl(TUNSETIFF)");
        close(fd);
        return err;
    }

    strcpy(dev, ifr.ifr_name);

    return fd;
}

int go_ping = 0;

void
umka_thread_ping(void) {
    umka_sti();
    while (!go_ping) { /* wait until initialized */ }

    f75ret_t r75;
    r75 = umka_sys_net_open_socket(AF_INET4, SOCK_STREAM, IPPROTO_TCP);
    if (r75.value == (uint32_t)-1) {
        fprintf(stderr, "[ping] open error %u\n", r75.errorcode);
        exit(1);
    }
    uint32_t sockfd = r75.value;

//    uint32_t addr = inet_addr("127.0.0.1");
//    uint32_t addr = inet_addr("192.243.108.5");
    uint32_t addr = inet_addr("10.50.0.1");
//    uint32_t addr = inet_addr("192.168.1.30");
    uint16_t port = 5000;

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET4;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = addr;

    r75 = umka_sys_net_connect(sockfd, &sa, sizeof(struct sockaddr_in));
    if (r75.value == (uint32_t)-1) {
        fprintf(stderr, "[ping] connect error %u\n", r75.errorcode);
        return;
    }

    r75 = umka_sys_net_send(sockfd, packet, 128, 0);
    if (r75.value == (uint32_t)-1) {
        fprintf(stderr, "[ping] send error %u\n", r75.errorcode);
        return;
    }

    while (true) {}

    return;
}

void
umka_thread_net_drv(void) {
    umka_sti();
    fprintf(stderr, "[net_drv] starting\n");
    int tapfd;
    uint8_t buffer[2048];
    int plen = 0;
    char tapdev[IFNAMSIZ] = "tap0";
    tapfd = tap_alloc(tapdev);
    net_device_t *vnet = vnet_init(tapfd);
    kos_net_add_device(vnet);

    char devname[64];
    for (size_t i = 0; i < umka_sys_net_get_dev_count(); i++) {
        umka_sys_net_dev_reset(i);
        umka_sys_net_get_dev_name(i, devname);
        uint32_t devtype = umka_sys_net_get_dev_type(i);
        fprintf(stderr, "[net_drv] device %i: %s %u\n", i, devname, devtype);
    }

    f76ret_t r76;
    r76 = umka_sys_net_ipv4_set_subnet(1, inet_addr("255.255.255.0"));
    if (r76.eax == (uint32_t)-1) {
        fprintf(stderr, "[net_drv] set subnet error\n");
        return;
    }

//    r76 = umka_sys_net_ipv4_set_gw(1, inet_addr("192.168.1.1"));
    r76 = umka_sys_net_ipv4_set_gw(1, inet_addr("10.50.0.1"));
    if (r76.eax == (uint32_t)-1) {
        fprintf(stderr, "set gw error\n");
        return;
    }

    r76 = umka_sys_net_ipv4_set_dns(1, inet_addr("217.10.36.5"));
    if (r76.eax == (uint32_t)-1) {
        fprintf(stderr, "[net_drv] set dns error\n");
        return;
    }

    r76 = umka_sys_net_ipv4_set_addr(1, inet_addr("10.50.0.2"));
    if (r76.eax == (uint32_t)-1) {
        fprintf(stderr, "[net_drv] set ip addr error\n");
        return;
    }

    go_ping = 1;

    while(true)
    {
        plen = read(tapfd, buffer, 2*1024);
        if (plen > 0) {
            fprintf(stderr, "[net_drv] read %i bytes\n", plen);
            for (int i = 0; i < plen; i++) {
                fprintf(stderr, " %2.2x", buffer[i]);
            }
            fprintf(stderr, "\n");
            vnet_receive_frame(vnet, buffer, plen);
        } else if(plen == -1 && (errno == EAGAIN || errno == EINTR)) {
            continue;
        } else {
            perror("[net_drv] reading data");
            exit(1);
        }
    }

}
