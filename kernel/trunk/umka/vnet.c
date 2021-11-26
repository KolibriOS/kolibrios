#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include "umka.h"
#include "trace.h"
#include "vnet.h"

typedef struct {
    int fd;
} vnet_userdata_t;

net_device_t *vnet_init(int fd) {
//    printf("vnet_init\n");
    vnet_userdata_t *u = (vnet_userdata_t*)malloc(sizeof(vnet_userdata_t));
    u->fd = fd;

    net_device_t *vnet = (net_device_t*)malloc(sizeof(net_device_t));
    *vnet = (net_device_t){
             .device_type = NET_TYPE_ETH,
             .mtu = 1514,
             .name = "UMK0770",

             .unload = vnet_unload,
             .reset = vnet_reset,
             .transmit = vnet_transmit,

             .bytes_tx = 0,
             .bytes_rx = 0,
             .packets_tx = 0,
             .packets_rx = 0,

             .link_state = ETH_LINK_FD + ETH_LINK_10M,
             .hwacc = 0,
             .mac = {0x80, 0x2b, 0xf9, 0x3b, 0x6c, 0xca},

             .userdata = u,
    };

    return vnet;
}

STDCALL void
vnet_unload() {
    printf("vnet_unload\n");
    COVERAGE_OFF();
    COVERAGE_ON();
}

STDCALL void
vnet_reset() {
    printf("vnet_reset\n");
    COVERAGE_OFF();
    COVERAGE_ON();
}

static void dump_net_buff(net_buff_t *buf) {
    for (size_t i = 0; i < buf->length; i++) {
        printf("%2.2x ", buf->data[i]);
    }
    putchar('\n');
}

STDCALL int
vnet_transmit(net_buff_t *buf) {
    net_device_t *vnet;
    __asm__ __inline__ __volatile__ (
        "nop"
        : "=b"(vnet)
        :
        : "memory");

    vnet_userdata_t *u = vnet->userdata;
    printf("vnet_transmit: %d bytes\n", buf->length);
    dump_net_buff(buf);
    write(u->fd, buf->data, buf->length);
    buf->length = 0;
    COVERAGE_OFF();
    COVERAGE_ON();
    printf("vnet_transmit: done\n");
    return 0;
}

void vnet_receive_frame(net_device_t *dev, void *data, size_t size) {
    net_buff_t *buf = kos_net_buff_alloc(size + offsetof(net_buff_t, data));
    if (!buf) {
        fprintf(stderr, "[vnet] Can't allocate network buffer!\n");
        return;
    }
    buf->length = size;
    buf->device = dev;
    buf->offset = offsetof(net_buff_t, data);
    memcpy(buf->data, data, size);
    __asm__ __inline__ __volatile__ (
        "pushad;"
        "lea    ecx, 1f;"
        "push   ecx;"
        "push   eax;"
        "jmp    kos_eth_input;"
        "1:"
        "popad"
        :
        : "a"(buf)
        : "memory", "ecx");
}
