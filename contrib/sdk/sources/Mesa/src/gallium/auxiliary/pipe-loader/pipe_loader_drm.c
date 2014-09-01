/**************************************************************************
 *
 * Copyright 2011 Intel Corporation
 * Copyright 2012 Francisco Jerez
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Kristian Høgsberg <krh@bitplanet.net>
 *    Benjamin Franzke <benjaminfranzke@googlemail.com>
 *
 **************************************************************************/

#include <fcntl.h>
#include <stdio.h>
//#include <libudev.h>
#include <xf86drm.h>

#ifdef HAVE_PIPE_LOADER_XCB

#include <xcb/dri2.h>

#endif

#include "state_tracker/drm_driver.h"
#include "pipe_loader_priv.h"

#include "util/u_memory.h"
#include "util/u_dl.h"
#include "util/u_debug.h"

#define DRIVER_MAP_GALLIUM_ONLY
#include "pci_ids/pci_id_driver_map.h"
#include <kos32sys.h>

struct pci_device {
    uint16_t    domain;
    uint8_t     bus;
    uint8_t     dev;
    uint8_t     func;
    uint16_t    vendor_id;
    uint16_t    device_id;
    uint16_t    subvendor_id;
    uint16_t    subdevice_id;
    uint32_t    device_class;
    uint8_t     revision;
};

struct pipe_loader_drm_device {
   struct pipe_loader_device base;
   struct util_dl_library *lib;
   int fd;
};

#define pipe_loader_drm_device(dev) ((struct pipe_loader_drm_device *)dev)

static boolean
find_drm_pci_id(struct pipe_loader_drm_device *ddev)
{
   struct pci_device device;
   ioctl_t   io;

   io.handle   = ddev->fd;
   io.io_code  = SRV_GET_PCI_INFO;
   io.input    = &device;
   io.inp_size = sizeof(device);
   io.output   = NULL;
   io.out_size = 0;

   if (call_service(&io)!=0)
      return FALSE;

   ddev->base.u.pci.vendor_id = device.vendor_id;
   ddev->base.u.pci.chip_id = device.device_id;

   return TRUE;
}

static boolean
find_drm_driver_name(struct pipe_loader_drm_device *ddev)
{
   struct pipe_loader_device *dev = &ddev->base;
   int i, j;

   for (i = 0; driver_map[i].driver; i++) {
      if (dev->u.pci.vendor_id != driver_map[i].vendor_id)
         continue;

      if (driver_map[i].num_chips_ids == -1) {
         dev->driver_name = driver_map[i].driver;
         goto found;
      }

      for (j = 0; j < driver_map[i].num_chips_ids; j++) {
         if (dev->u.pci.chip_id == driver_map[i].chip_ids[j]) {
            dev->driver_name = driver_map[i].driver;
            goto found;
         }
      }
   }

   return FALSE;

  found:
   return TRUE;
}

static struct pipe_loader_ops pipe_loader_drm_ops;


boolean
pipe_loader_drm_probe_fd(struct pipe_loader_device **dev, int fd)
{
   struct pipe_loader_drm_device *ddev = CALLOC_STRUCT(pipe_loader_drm_device);

   ddev->base.type = PIPE_LOADER_DEVICE_PCI;
   ddev->base.ops = &pipe_loader_drm_ops;
   ddev->fd = fd;

   if (!find_drm_pci_id(ddev))
      goto fail;

   if (!find_drm_driver_name(ddev))
      goto fail;

   *dev = &ddev->base;
   return TRUE;

  fail:
   FREE(ddev);
   return FALSE;
}


int
pipe_loader_drm_probe(struct pipe_loader_device **devs, int ndev)
{
   int i, j, fd;

   for (i = 0, j = 0; i < 1; i++) {
      fd = get_service("DISPLAY");
      if (fd == 0)
         continue;

      if (j >= ndev || !pipe_loader_drm_probe_fd(&devs[j], fd))
         ;

      j++;
   }

   return j;
}

static void
pipe_loader_drm_release(struct pipe_loader_device **dev)
{
   struct pipe_loader_drm_device *ddev = pipe_loader_drm_device(*dev);

   FREE(ddev);
   *dev = NULL;
}

static struct pipe_screen *
pipe_loader_drm_create_screen(struct pipe_loader_device *dev,
                              const char *library_paths)
{
   struct pipe_loader_drm_device *ddev = pipe_loader_drm_device(dev);
   const struct drm_driver_descriptor *dd;

   if (!ddev->lib)
      ddev->lib = pipe_loader_find_module(dev, library_paths);
   if (!ddev->lib)
      return NULL;

   dd = (const struct drm_driver_descriptor *)
      util_dl_get_proc_address(ddev->lib, "driver_descriptor");

   /* sanity check on the name */
   if (!dd || strcmp(dd->name, ddev->base.driver_name) != 0)
      return NULL;

   return dd->create_screen(ddev->fd);
}

static struct pipe_loader_ops pipe_loader_drm_ops = {
   .create_screen = pipe_loader_drm_create_screen,
   .release = pipe_loader_drm_release
};
