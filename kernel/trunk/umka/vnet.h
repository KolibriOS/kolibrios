#ifndef VNET_H_INCLUDED
#define VNET_H_INCLUDED

#include <stdio.h>
#include <inttypes.h>
#include "umka.h"

net_device_t *vnet_init(int fd);

STDCALL void
vnet_unload(void);

STDCALL void
vnet_reset(void);

STDCALL int
vnet_transmit(net_buff_t *buf);

void
vnet_receive_frame(net_device_t *dev, void *data, size_t size);

#endif  // VNET_H_INCLUDED
