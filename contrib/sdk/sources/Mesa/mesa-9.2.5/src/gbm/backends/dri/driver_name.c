/*
 * Copyright © 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Kristian Høgsberg <krh@bitplanet.net>
 *    Benjamin Franzke <benjaminfranzke@googlemail.com>
 */

#include <stdio.h>
#include <string.h>
#include <pciaccess.h>
#include <kos32sys.h>

#include "gbm_driint.h"
#define DRIVER_MAP_DRI2_ONLY
#include "pci_ids/pci_id_driver_map.h"

char *
dri_fd_get_driver_name(int fd)
{
    ioctl_t   io;
    struct pci_device device;
   char *driver = NULL;
    int i, j;

    io.handle   = fd;
    io.io_code  = SRV_GET_PCI_INFO;
    io.input    = &device;
    io.inp_size = sizeof(device);
    io.output   = NULL;
    io.out_size = 0;

    if (call_service(&io)!=0)
      return NULL;

    for (i = 0; driver_map[i].driver; i++)
    {
        if (device.vendor_id != driver_map[i].vendor_id)
         continue;
        if (driver_map[i].num_chips_ids == -1)
        {
         driver = strdup(driver_map[i].driver);
            printf("pci id for %d: %04x:%04x, driver %s\n",
                  fd, device.vendor_id, device.device_id, driver);
         goto out;
      }

      for (j = 0; j < driver_map[i].num_chips_ids; j++)
            if (driver_map[i].chip_ids[j] == device.device_id)
            {
            driver = strdup(driver_map[i].driver);
                printf("pci id for %d: %04x:%04x, driver %s\n",
                     fd, device.vendor_id, device.device_id, driver);
            goto out;
         }
   }

out:

   return driver;
}
