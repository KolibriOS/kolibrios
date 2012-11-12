/*
 * Copyright 2008 Intel Corporation <hong.liu@intel.com>
 * Copyright 2008 Red Hat <mjg@redhat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL INTEL AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

//#include <linux/acpi.h>
//#include <linux/acpi_io.h>
//#include <acpi/video.h>
#include <linux/errno.h>


#include <drm/drmP.h>
#include <drm/i915_drm.h>
#include "i915_drv.h"
#include "intel_drv.h"

#define PCI_ASLE 0xe4
#define PCI_ASLS 0xfc

#define OPREGION_HEADER_OFFSET 0
#define OPREGION_ACPI_OFFSET   0x100
#define   ACPI_CLID 0x01ac /* current lid state indicator */
#define   ACPI_CDCK 0x01b0 /* current docking state indicator */
#define OPREGION_SWSCI_OFFSET  0x200
#define OPREGION_ASLE_OFFSET   0x300
#define OPREGION_VBT_OFFSET    0x400

#define OPREGION_SIGNATURE "IntelGraphicsMem"
#define MBOX_ACPI      (1<<0)
#define MBOX_SWSCI     (1<<1)
#define MBOX_ASLE      (1<<2)

struct opregion_header {
       u8 signature[16];
       u32 size;
       u32 opregion_ver;
       u8 bios_ver[32];
       u8 vbios_ver[16];
       u8 driver_ver[16];
       u32 mboxes;
       u8 reserved[164];
} __attribute__((packed));

/* OpRegion mailbox #1: public ACPI methods */
struct opregion_acpi {
       u32 drdy;       /* driver readiness */
       u32 csts;       /* notification status */
       u32 cevt;       /* current event */
       u8 rsvd1[20];
       u32 didl[8];    /* supported display devices ID list */
       u32 cpdl[8];    /* currently presented display list */
       u32 cadl[8];    /* currently active display list */
       u32 nadl[8];    /* next active devices list */
       u32 aslp;       /* ASL sleep time-out */
       u32 tidx;       /* toggle table index */
       u32 chpd;       /* current hotplug enable indicator */
       u32 clid;       /* current lid state*/
       u32 cdck;       /* current docking state */
       u32 sxsw;       /* Sx state resume */
       u32 evts;       /* ASL supported events */
       u32 cnot;       /* current OS notification */
       u32 nrdy;       /* driver status */
       u8 rsvd2[60];
} __attribute__((packed));

/* OpRegion mailbox #2: SWSCI */
struct opregion_swsci {
       u32 scic;       /* SWSCI command|status|data */
       u32 parm;       /* command parameters */
       u32 dslp;       /* driver sleep time-out */
       u8 rsvd[244];
} __attribute__((packed));

/* OpRegion mailbox #3: ASLE */
struct opregion_asle {
       u32 ardy;       /* driver readiness */
       u32 aslc;       /* ASLE interrupt command */
       u32 tche;       /* technology enabled indicator */
       u32 alsi;       /* current ALS illuminance reading */
       u32 bclp;       /* backlight brightness to set */
       u32 pfit;       /* panel fitting state */
       u32 cblv;       /* current brightness level */
       u16 bclm[20];   /* backlight level duty cycle mapping table */
       u32 cpfm;       /* current panel fitting mode */
       u32 epfm;       /* enabled panel fitting modes */
       u8 plut[74];    /* panel LUT and identifier */
       u32 pfmb;       /* PWM freq and min brightness */
       u8 rsvd[102];
} __attribute__((packed));

/* ASLE irq request bits */
#define ASLE_SET_ALS_ILLUM     (1 << 0)
#define ASLE_SET_BACKLIGHT     (1 << 1)
#define ASLE_SET_PFIT          (1 << 2)
#define ASLE_SET_PWM_FREQ      (1 << 3)
#define ASLE_REQ_MSK           0xf

/* response bits of ASLE irq request */
#define ASLE_ALS_ILLUM_FAILED	(1<<10)
#define ASLE_BACKLIGHT_FAILED	(1<<12)
#define ASLE_PFIT_FAILED	(1<<14)
#define ASLE_PWM_FREQ_FAILED	(1<<16)

/* ASLE backlight brightness to set */
#define ASLE_BCLP_VALID                (1<<31)
#define ASLE_BCLP_MSK          (~(1<<31))

/* ASLE panel fitting request */
#define ASLE_PFIT_VALID         (1<<31)
#define ASLE_PFIT_CENTER (1<<0)
#define ASLE_PFIT_STRETCH_TEXT (1<<1)
#define ASLE_PFIT_STRETCH_GFX (1<<2)

/* PWM frequency and minimum brightness */
#define ASLE_PFMB_BRIGHTNESS_MASK (0xff)
#define ASLE_PFMB_BRIGHTNESS_VALID (1<<8)
#define ASLE_PFMB_PWM_MASK (0x7ffffe00)
#define ASLE_PFMB_PWM_VALID (1<<31)

#define ASLE_CBLV_VALID         (1<<31)

#define ACPI_OTHER_OUTPUT (0<<8)
#define ACPI_VGA_OUTPUT (1<<8)
#define ACPI_TV_OUTPUT (2<<8)
#define ACPI_DIGITAL_OUTPUT (3<<8)
#define ACPI_LVDS_OUTPUT (4<<8)

#ifdef CONFIG_ACPI
#endif

int intel_opregion_setup(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_opregion *opregion = &dev_priv->opregion;
	void __iomem *base;
	u32 asls, mboxes;
	char buf[sizeof(OPREGION_SIGNATURE)];
	int err = 0;

	pci_read_config_dword(dev->pdev, PCI_ASLS, &asls);
	DRM_DEBUG_DRIVER("graphic opregion physical addr: 0x%x\n", asls);
	if (asls == 0) {
		DRM_DEBUG_DRIVER("ACPI OpRegion not supported!\n");
		return -ENOTSUPP;
	}

    base = ioremap(asls, OPREGION_SIZE);
	if (!base)
		return -ENOMEM;

	memcpy(buf, base, sizeof(buf));

	if (memcmp(buf, OPREGION_SIGNATURE, 16)) {
		DRM_DEBUG_DRIVER("opregion signature mismatch\n");
		err = -EINVAL;
		goto err_out;
	}
	opregion->header = base;
	opregion->vbt = base + OPREGION_VBT_OFFSET;

	opregion->lid_state = base + ACPI_CLID;

	mboxes = ioread32(&opregion->header->mboxes);
	if (mboxes & MBOX_ACPI) {
		DRM_DEBUG_DRIVER("Public ACPI methods supported\n");
		opregion->acpi = base + OPREGION_ACPI_OFFSET;
	}

	if (mboxes & MBOX_SWSCI) {
		DRM_DEBUG_DRIVER("SWSCI supported\n");
		opregion->swsci = base + OPREGION_SWSCI_OFFSET;
	}
	if (mboxes & MBOX_ASLE) {
		DRM_DEBUG_DRIVER("ASLE supported\n");
		opregion->asle = base + OPREGION_ASLE_OFFSET;
	}

	return 0;

err_out:
	iounmap(base);
	return err;
}
