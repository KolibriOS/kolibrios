/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2012-2013 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include "genhw/genhw.h"
#include "intel_winsys.h"

#include "ilo_debug.h"
#include "ilo_dev.h"

/**
 * Initialize the \p dev from \p winsys.  \p winsys is considered owned by \p
 * dev and will be destroyed in \p ilo_dev_cleanup().
 */
bool
ilo_dev_init(struct ilo_dev *dev, struct intel_winsys *winsys)
{
   const struct intel_winsys_info *info;

   info = intel_winsys_get_info(winsys);

   dev->winsys = winsys;
   dev->devid = info->devid;
   dev->aperture_total = info->aperture_total;
   dev->aperture_mappable = info->aperture_mappable;
   dev->has_llc = info->has_llc;
   dev->has_address_swizzling = info->has_address_swizzling;
   dev->has_logical_context = info->has_logical_context;
   dev->has_ppgtt = info->has_ppgtt;
   dev->has_timestamp = info->has_timestamp;
   dev->has_gen7_sol_reset = info->has_gen7_sol_reset;

   if (!dev->has_logical_context) {
      ilo_err("missing hardware logical context support\n");
      return false;
   }

   /*
    * PIPE_CONTROL and MI_* use PPGTT writes on GEN7+ and privileged GGTT
    * writes on GEN6.
    *
    * From the Sandy Bridge PRM, volume 1 part 3, page 101:
    *
    *     "[DevSNB] When Per-Process GTT Enable is set, it is assumed that all
    *      code is in a secure environment, independent of address space.
    *      Under this condition, this bit only specifies the address space
    *      (GGTT or PPGTT). All commands are executed "as-is""
    *
    * We need PPGTT to be enabled on GEN6 too.
    */
   if (!dev->has_ppgtt) {
      /* experiments show that it does not really matter... */
      ilo_warn("PPGTT disabled\n");
   }

   if (gen_is_bdw(info->devid) || gen_is_chv(info->devid)) {
      dev->gen_opaque = ILO_GEN(8);
      dev->gt = (gen_is_bdw(info->devid)) ? gen_get_bdw_gt(info->devid) : 1;
      /* XXX random values */
      if (dev->gt == 3) {
         dev->eu_count = 48;
         dev->thread_count = 336;
         dev->urb_size = 384 * 1024;
      } else if (dev->gt == 2) {
         dev->eu_count = 24;
         dev->thread_count = 168;
         dev->urb_size = 384 * 1024;
      } else {
         dev->eu_count = 12;
         dev->thread_count = 84;
         dev->urb_size = 192 * 1024;
      }
   } else if (gen_is_hsw(info->devid)) {
      /*
       * From the Haswell PRM, volume 4, page 8:
       *
       *     "Description                    GT3      GT2      GT1.5    GT1
       *      (...)
       *      EUs (Total)                    40       20       12       10
       *      Threads (Total)                280      140      84       70
       *      (...)
       *      URB Size (max, within L3$)     512KB    256KB    256KB    128KB
       */
      dev->gen_opaque = ILO_GEN(7.5);
      dev->gt = gen_get_hsw_gt(info->devid);
      if (dev->gt == 3) {
         dev->eu_count = 40;
         dev->thread_count = 280;
         dev->urb_size = 512 * 1024;
      } else if (dev->gt == 2) {
         dev->eu_count = 20;
         dev->thread_count = 140;
         dev->urb_size = 256 * 1024;
      } else {
         dev->eu_count = 10;
         dev->thread_count = 70;
         dev->urb_size = 128 * 1024;
      }
   } else if (gen_is_ivb(info->devid) || gen_is_vlv(info->devid)) {
      /*
       * From the Ivy Bridge PRM, volume 1 part 1, page 18:
       *
       *     "Device             # of EUs        #Threads/EU
       *      Ivy Bridge (GT2)   16              8
       *      Ivy Bridge (GT1)   6               6"
       *
       * From the Ivy Bridge PRM, volume 4 part 2, page 17:
       *
       *     "URB Size    URB Rows    URB Rows when SLM Enabled
       *      128k        4096        2048
       *      256k        8096        4096"
       */
      dev->gen_opaque = ILO_GEN(7);
      dev->gt = (gen_is_ivb(info->devid)) ? gen_get_ivb_gt(info->devid) : 1;
      if (dev->gt == 2) {
         dev->eu_count = 16;
         dev->thread_count = 128;
         dev->urb_size = 256 * 1024;
      } else {
         dev->eu_count = 6;
         dev->thread_count = 36;
         dev->urb_size = 128 * 1024;
      }
   } else if (gen_is_snb(info->devid)) {
      /*
       * From the Sandy Bridge PRM, volume 1 part 1, page 22:
       *
       *     "Device             # of EUs        #Threads/EU
       *      SNB GT2            12              5
       *      SNB GT1            6               4"
       *
       * From the Sandy Bridge PRM, volume 4 part 2, page 18:
       *
       *     "[DevSNB]: The GT1 product's URB provides 32KB of storage,
       *      arranged as 1024 256-bit rows. The GT2 product's URB provides
       *      64KB of storage, arranged as 2048 256-bit rows. A row
       *      corresponds in size to an EU GRF register. Read/write access to
       *      the URB is generally supported on a row-granular basis."
       */
      dev->gen_opaque = ILO_GEN(6);
      dev->gt = gen_get_snb_gt(info->devid);
      if (dev->gt == 2) {
         dev->eu_count = 12;
         dev->thread_count = 60;
         dev->urb_size = 64 * 1024;
      } else {
         dev->eu_count = 6;
         dev->thread_count = 24;
         dev->urb_size = 32 * 1024;
      }
   } else {
      ilo_err("unknown GPU generation\n");
      return false;
   }

   return true;
}

void
ilo_dev_cleanup(struct ilo_dev *dev)
{
   intel_winsys_destroy(dev->winsys);
}
