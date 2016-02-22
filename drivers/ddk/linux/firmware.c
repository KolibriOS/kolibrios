
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/byteorder/little_endian.h>
#include <linux/gfp.h>
#include <linux/errno.h>
#include <linux/firmware.h>

extern struct builtin_fw __start_builtin_fw[];
extern struct builtin_fw __end_builtin_fw[];

/* Intel HEX files actually limit the length to 256 bytes, but we have
   drivers which would benefit from using separate records which are
   longer than that, so we extend to 16 bits of length */
struct ihex_binrec {
    __be32      addr;
    __be16      len;
    uint8_t     data[0];
} __attribute__((packed));

/* Find the next record, taking into account the 4-byte alignment */
static inline const struct ihex_binrec *
ihex_next_binrec(const struct ihex_binrec *rec)
{
    int next = ((be16_to_cpu(rec->len) + 5) & ~3) - 2;
    rec = (void *)&rec->data[next];

    return be16_to_cpu(rec->len) ? rec : NULL;
}

int
request_firmware(const struct firmware **firmware_p, const char *name,
                 struct device *device)
{

    struct firmware *firmware;
    struct builtin_fw *builtin;
    const struct ihex_binrec *rec;
    unsigned int size;

    int retval;

    if (!firmware_p)
        return -EINVAL;

    *firmware_p = firmware = kzalloc(sizeof(*firmware), GFP_KERNEL);
    if (!firmware) {
        dbgprintf("%s: kmalloc(struct firmware) failed\n", __func__);
        return  -ENOMEM;
    }

    for (builtin = __start_builtin_fw; builtin != __end_builtin_fw;
         builtin++)
    {
        uint8_t  *pfw;

        if (strcmp(name, builtin->name))
            continue;
        dbgprintf("firmware: using built-in firmware %s\n", name);

#if 0
        size = 0;
        for (rec = (const struct ihex_binrec *)builtin->data;
             rec; rec = ihex_next_binrec(rec))
        {
            size += be16_to_cpu(rec->len);
        }
        dbgprintf("firmware size %d\n", size);

        if(unlikely( size == 0))
            return -EINVAL;


        pfw = (uint8_t*)kzalloc(size, 0);

        if(unlikely(pfw == 0))
            return -ENOMEM;

        firmware->size = size;
        firmware->data = pfw;

        for (rec = (const struct ihex_binrec *)builtin->data;
             rec; rec = ihex_next_binrec(rec))
        {
            unsigned int src_size;

            src_size = be16_to_cpu(rec->len);
            memcpy(pfw, rec->data, src_size);
            pfw+= src_size;
        };
#else
        dbgprintf("firmware size %d\n", builtin->size);

        firmware->size = builtin->size;
        firmware->data = builtin->data;
#endif
        return 0;
    }

    kfree(firmware);
    *firmware_p = NULL;

    return -EINVAL;
};

void
release_firmware(const struct firmware *fw)
{
    if (fw) {
        kfree((void*)fw);
    }
}
/*
struct platform_device*
platform_device_register_simple(const char* c, int id, void *r, unsigned int i)
{
    static struct platform_device pd;

    return &pd;
};
*/

