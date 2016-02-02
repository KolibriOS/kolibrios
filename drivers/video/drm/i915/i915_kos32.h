
#define OPTION_STD_BASE 256

enum option_values
{
      OPTION_HELP = OPTION_STD_BASE,
      OPTION_RC6,
      OPTION_FBC,
      OPTION_CONNECTORS,
      OPTION_CONN_MODES
};


typedef union __attribute__((packed))
{
    uint32_t val;
    struct
    {
        uint8_t   state;
        uint8_t   code;
        uint16_t  ctrl_key;
    };
}oskey_t;

static inline oskey_t get_key(void)
{
    oskey_t val;
    asm volatile("int $0x40":"=a"(val):"a"(2));
    return val;
};

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
void get_pci_info(struct pci_device *dev);

int split_cmdline(char *cmdline, char **argv);

int do_command_line(const char* usercmd);

int set_cmdline_mode_ext(struct drm_device *dev, const char *cmdline);
void list_connectors(struct drm_device *dev);
int list_connector_modes(struct drm_device *dev, const char* name);

int _stdcall display_handler(ioctl_t *io);

void i915_dpms(struct drm_device *dev, int mode);
int i915_getparam(struct drm_device *dev, void *data,
             struct drm_file *file_priv);
int i915_mask_update(struct drm_device *dev, void *data,
            struct drm_file *file);
