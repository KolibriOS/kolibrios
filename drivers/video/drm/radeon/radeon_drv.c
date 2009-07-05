
#include "radeon_reg.h"
#include "radeon.h"
#include "radeon_asic.h"
#include "syscall.h"

int radeon_dynclks = -1;

static struct pci_device_id pciidlist[] = {
    radeon_PCI_IDS
};

static struct drm_driver kms_driver = {
    .driver_features =
        DRIVER_USE_AGP | DRIVER_USE_MTRR | DRIVER_PCI_DMA | DRIVER_SG |
        DRIVER_HAVE_IRQ | DRIVER_HAVE_DMA | DRIVER_IRQ_SHARED | DRIVER_GEM,
    .dev_priv_size = 0,
    .load = radeon_driver_load_kms,
    .firstopen = radeon_driver_firstopen_kms,
    .open = radeon_driver_open_kms,
    .preclose = radeon_driver_preclose_kms,
    .postclose = radeon_driver_postclose_kms,
    .lastclose = radeon_driver_lastclose_kms,
    .unload = radeon_driver_unload_kms,
    .suspend = radeon_suspend_kms,
    .resume = radeon_resume_kms,
    .get_vblank_counter = radeon_get_vblank_counter_kms,
    .enable_vblank = radeon_enable_vblank_kms,
    .disable_vblank = radeon_disable_vblank_kms,
    .master_create = radeon_master_create_kms,
    .master_destroy = radeon_master_destroy_kms,
    .irq_preinstall = radeon_driver_irq_preinstall_kms,
    .irq_postinstall = radeon_driver_irq_postinstall_kms,
    .irq_uninstall = radeon_driver_irq_uninstall_kms,
    .irq_handler = radeon_driver_irq_handler_kms,
    .reclaim_buffers = drm_core_reclaim_buffers,
    .get_map_ofs = drm_core_get_map_ofs,
    .get_reg_ofs = drm_core_get_reg_ofs,
    .ioctls = radeon_ioctls_kms,
    .gem_init_object = radeon_gem_object_init,
    .gem_free_object = radeon_gem_object_free,
    .dma_ioctl = radeon_dma_ioctl_kms,
    .fops = {
         .owner = THIS_MODULE,
         .open = drm_open,
         .release = drm_release,
         .ioctl = drm_ioctl,
         .mmap = radeon_mmap,
         .poll = drm_poll,
         .fasync = drm_fasync,
    },

    .pci_driver = {
         .name = DRIVER_NAME,
         .id_table = pciidlist,
         .probe = radeon_pci_probe,
         .remove = radeon_pci_remove,
         .suspend = radeon_pci_suspend,
         .resume = radeon_pci_resume,
    },

    .name = DRIVER_NAME,
    .desc = DRIVER_DESC,
    .date = DRIVER_DATE,
    .major = KMS_DRIVER_MAJOR,
    .minor = KMS_DRIVER_MINOR,
    .patchlevel = KMS_DRIVER_PATCHLEVEL,
};


static int __init radeon_init(void)
{
    radeon_modeset = 1;
    driver = &kms_driver;
    driver->driver_features |= DRIVER_MODESET;
    driver->num_ioctls = radeon_max_kms_ioctl;

    return drm_init(driver);
}

struct pci_driver
{
    struct list_head node;
    char *name;
    const struct pci_device_id *id_table;   /* must be non-NULL for probe to be called */
    int  (*probe)  (struct pci_dev *dev, const struct pci_device_id *id);   /* New device inserted */
    void (*remove) (struct pci_dev *dev);   /* Device removed (NULL if not a hot-plug capable driver) */
    int  (*suspend) (struct pci_dev *dev, pm_message_t state);      /* Device suspended */
    int  (*suspend_late) (struct pci_dev *dev, pm_message_t state);
    int  (*resume_early) (struct pci_dev *dev);
    int  (*resume) (struct pci_dev *dev);                   /* Device woken up */
    void (*shutdown) (struct pci_dev *dev);

    struct pci_error_handlers *err_handler;
    struct device_driver    driver;
    struct pci_dynids dynids;
};

