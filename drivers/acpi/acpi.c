
#include <syscall.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/acpi.h>
#include <linux/dmi.h>


#define PREFIX          "ACPI: "

int sbf_port __initdata = -1;

static bool acpi_os_initialized;


u32  __attribute__((externally_visible)) drvEntry(int action, char *cmdline)
{
    int result;

    if(action != 1)
        return 0;

    if( !dbg_open("/tmp0/1/acpi.log") )
    {
        printk("Can't open /tmp0/1/acpi.log\nExit\n");
        return 0;
    }

    dmi_scan_machine();

    acpi_boot_table_init();

    early_acpi_boot_init();

    acpi_noirq_set();

    acpi_early_init();


//    if (acpi_disabled) {
//            printk(KERN_INFO PREFIX "Interpreter disabled.\n");
//            return -ENODEV;
//    }

//    init_acpi_device_notify();
//    result = acpi_bus_init();
//    if (result) {
//            disable_acpi();
//            return result;
//    }

//    pci_mmcfg_late_init();
//    acpi_scan_init();
//    acpi_ec_init();
//    acpi_debugfs_init();
//    acpi_sleep_proc_init();
//    acpi_wakeup_device_init();

    dbgprintf("module loaded\n");


    return 0;

};


#define PREFIX          "ACPI: "

u32 IMPORT  AcpiGetRootPtr(void)__asm__("AcpiGetRootPtr");

acpi_physical_address __init acpi_os_get_root_pointer(void)
{
    return AcpiGetRootPtr();
}

void* acpi_os_map_memory(acpi_physical_address phys, acpi_size size)
{
    return (void *)MapIoMem((addr_t)phys, size, PG_SW);

}

void acpi_os_unmap_memory(void *virt, acpi_size size)
{
    u32 ptr = (u32)virt;
    ptr &= 0xFFFFF000;

    return FreeKernelSpace((void*)ptr);
}

void __init early_acpi_os_unmap_memory(void __iomem *virt, acpi_size size)
{
    acpi_os_unmap_memory(virt, size);
}


int acpi_os_map_generic_address(struct acpi_generic_address *gas)
{
    addr_t addr;
    void *virt;

    if (gas->space_id != ACPI_ADR_SPACE_SYSTEM_MEMORY)
        return 0;

    addr = (addr_t)gas->address;

    if (!addr || !gas->bit_width)
        return -EINVAL;

    virt = (void *)MapIoMem(addr, gas->bit_width / 8, PG_SW);
    if (!virt)
        return -EIO;

    return 0;
}


void acpi_os_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    acpi_os_vprintf(fmt, args);
    va_end(args);
}

void acpi_os_vprintf(const char *fmt, va_list args)
{
    static char buffer[512];

    vsprintf(buffer, fmt, args);

#ifdef ENABLE_DEBUGGER
    if (acpi_in_debugger) {
        kdb_printf("%s", buffer);
    } else {
            printk("%s", buffer);
    }
#else
    printk("%s", buffer);
#endif
}


static void acpi_table_taint(struct acpi_table_header *table)
{
     pr_warn(PREFIX
             "Override [%4.4s-%8.8s], this is unsafe: tainting kernel\n",
             table->signature, table->oem_table_id);
     add_taint(TAINT_OVERRIDDEN_ACPI_TABLE, LOCKDEP_NOW_UNRELIABLE);
}




acpi_status
acpi_os_table_override(struct acpi_table_header * existing_table,
                       struct acpi_table_header ** new_table)
{
    if (!existing_table || !new_table)
        return AE_BAD_PARAMETER;

    *new_table = NULL;

#ifdef CONFIG_ACPI_CUSTOM_DSDT
    if (strncmp(existing_table->signature, "DSDT", 4) == 0)
        *new_table = (struct acpi_table_header *)AmlCode;
#endif
    if (*new_table != NULL)
        acpi_table_taint(existing_table);
    return AE_OK;
}

acpi_status
acpi_os_physical_table_override(struct acpi_table_header *existing_table,
                                acpi_physical_address *address,
                                u32 *table_length)
{
#ifndef CONFIG_ACPI_INITRD_TABLE_OVERRIDE
        *table_length = 0;
        *address = 0;
        return AE_OK;
#else
        int table_offset = 0;
        struct acpi_table_header *table;

        *table_length = 0;
        *address = 0;

        if (!acpi_tables_addr)
                return AE_OK;

        do {
                if (table_offset + ACPI_HEADER_SIZE > all_tables_size) {
                        WARN_ON(1);
                        return AE_OK;
                }

                table = acpi_os_map_memory(acpi_tables_addr + table_offset,
                                           ACPI_HEADER_SIZE);

                if (table_offset + table->length > all_tables_size) {
                        acpi_os_unmap_memory(table, ACPI_HEADER_SIZE);
                        WARN_ON(1);
                        return AE_OK;
                }

                table_offset += table->length;

                if (memcmp(existing_table->signature, table->signature, 4)) {
                        acpi_os_unmap_memory(table,
                                     ACPI_HEADER_SIZE);
                        continue;
                }

                /* Only override tables with matching oem id */
                if (memcmp(table->oem_table_id, existing_table->oem_table_id,
                           ACPI_OEM_TABLE_ID_SIZE)) {
                        acpi_os_unmap_memory(table,
                                     ACPI_HEADER_SIZE);
                        continue;
                }

                table_offset -= table->length;
                *table_length = table->length;
                acpi_os_unmap_memory(table, ACPI_HEADER_SIZE);
                *address = acpi_tables_addr + table_offset;
                break;
        } while (table_offset + ACPI_HEADER_SIZE < all_tables_size);

        if (*address != 0)
                acpi_table_taint(existing_table);
        return AE_OK;
#endif
}


static struct osi_linux {
        unsigned int    enable:1;
        unsigned int    dmi:1;
        unsigned int    cmdline:1;
        unsigned int    default_disabling:1;

} osi_linux = {0, 0, 0, 0};

#define OSI_STRING_LENGTH_MAX 64        /* arbitrary */
#define OSI_STRING_ENTRIES_MAX 16       /* arbitrary */

struct osi_setup_entry {
    char string[OSI_STRING_LENGTH_MAX];
    bool enable;
};

static struct osi_setup_entry
        osi_setup_entries[OSI_STRING_ENTRIES_MAX] __initdata = {
    {"Module Device", true},
    {"Processor Device", true},
    {"3.0 _SCP Extensions", true},
    {"Processor Aggregator Device", true},
};
void __init acpi_osi_setup(char *str)
{
        struct osi_setup_entry *osi;
        bool enable = true;
        int i;

        if (!acpi_gbl_create_osi_method)
                return;

        if (str == NULL || *str == '\0') {
                printk(KERN_INFO PREFIX "_OSI method disabled\n");
                acpi_gbl_create_osi_method = FALSE;
                return;
        }

        if (*str == '!') {
                str++;
                if (*str == '\0') {
                        osi_linux.default_disabling = 1;
                        return;
                } else if (*str == '*') {
                        acpi_update_interfaces(ACPI_DISABLE_ALL_STRINGS);
                        for (i = 0; i < OSI_STRING_ENTRIES_MAX; i++) {
                                osi = &osi_setup_entries[i];
                                osi->enable = false;
                        }
                        return;
                }
                enable = false;
        }

        for (i = 0; i < OSI_STRING_ENTRIES_MAX; i++) {
                osi = &osi_setup_entries[i];
                if (!strcmp(osi->string, str)) {
                        osi->enable = enable;
                        break;
                } else if (osi->string[0] == '\0') {
                        osi->enable = enable;
                        strncpy(osi->string, str, OSI_STRING_LENGTH_MAX);
                        break;
                }
        }
}

static void __init set_osi_linux(unsigned int enable)
{
    if (osi_linux.enable != enable)
        osi_linux.enable = enable;

    if (osi_linux.enable)
        acpi_osi_setup("Linux");
    else
        acpi_osi_setup("!Linux");

    return;
}

void __init acpi_dmi_osi_linux(int enable, const struct dmi_system_id *d)
{
    printk(KERN_NOTICE PREFIX "DMI detected: %s\n", d->ident);

    if (enable == -1)
        return;

    osi_linux.dmi = 1;  /* DMI knows that this box asks OSI(Linux) */
    set_osi_linux(enable);

    return;
}

acpi_status acpi_os_wait_semaphore(acpi_handle handle, u32 units, u16 timeout)
{
    void *sem = (void*)handle;

    if (!acpi_os_initialized)
        return AE_OK;

    if (!sem || (units < 1))
        return AE_BAD_PARAMETER;

    if (units > 1)
        return AE_SUPPORT;

    ACPI_DEBUG_PRINT((ACPI_DB_MUTEX, "Waiting for semaphore[%p|%d|%d]\n",
                      handle, units, timeout));

    return AE_OK;
}

acpi_status acpi_os_signal_semaphore(acpi_handle handle, u32 units)
{
    void *sem = (void*)handle;

    if (!acpi_os_initialized)
            return AE_OK;

    if (!sem || (units < 1))
            return AE_BAD_PARAMETER;

    if (units > 1)
            return AE_SUPPORT;

    ACPI_DEBUG_PRINT((ACPI_DB_MUTEX, "Signaling semaphore[%p|%d]\n", handle,
                      units));

//    up(sem);

    return AE_OK;
}


acpi_status __init acpi_os_initialize(void)
{
//    acpi_os_map_generic_address(&acpi_gbl_FADT.xpm1a_event_block);
//    acpi_os_map_generic_address(&acpi_gbl_FADT.xpm1b_event_block);
//    acpi_os_map_generic_address(&acpi_gbl_FADT.xgpe0_block);
//    acpi_os_map_generic_address(&acpi_gbl_FADT.xgpe1_block);
    if (acpi_gbl_FADT.flags & ACPI_FADT_RESET_REGISTER) {
            /*
             * Use acpi_os_map_generic_address to pre-map the reset
             * register if it's in system memory.
             */
            int rv;

            rv = acpi_os_map_generic_address(&acpi_gbl_FADT.reset_register);
            pr_debug(PREFIX "%s: map reset_reg status %d\n", __func__, rv);
    }
    acpi_os_initialized = true;

    return AE_OK;
}

acpi_status __init acpi_os_initialize1(void)
{
//        kacpid_wq = alloc_workqueue("kacpid", 0, 1);
//        kacpi_notify_wq = alloc_workqueue("kacpi_notify", 0, 1);
//        kacpi_hotplug_wq = alloc_ordered_workqueue("kacpi_hotplug", 0);
//        BUG_ON(!kacpid_wq);
//        BUG_ON(!kacpi_notify_wq);
//       BUG_ON(!kacpi_hotplug_wq);
//        acpi_install_interface_handler(acpi_osi_handler);
//        acpi_osi_setup_late();
        return AE_OK;
}


acpi_status acpi_os_delete_semaphore(acpi_handle handle)
{
//        void *sem = (void*)handle;

//        if (!sem)
//            return AE_BAD_PARAMETER;

//        kfree(sem);
//        sem = NULL;

        return AE_OK;
}

acpi_status acpi_os_create_semaphore(u32 max_units,
                         u32 initial_units, acpi_handle * out_handle)
{
    *out_handle = (acpi_handle) 1;
    return (AE_OK);
}

acpi_status
acpi_os_create_mutex(acpi_handle * handle)
{
    struct mutex *mtx = NULL;

    mtx = kzalloc(sizeof(struct mutex),0);

    if (!mtx)
            return AE_NO_MEMORY;

    mutex_init(mtx);

    *handle = (acpi_handle *) mtx;

     return AE_OK;
}

void acpi_os_release_mutex(acpi_mutex handle)
{
    struct mutex *mtx = (struct mutex*)handle;

    if (!acpi_os_initialized)
        return;

    mutex_unlock(mtx);
}

acpi_status acpi_os_acquire_mutex(acpi_mutex handle, u16 timeout)
{
    struct mutex *mtx = (struct mutex*)handle;

    if (!acpi_os_initialized)
        return AE_OK;

    mutex_lock(mtx);

    return AE_OK;
}

void acpi_os_delete_mutex(acpi_mutex handle)
{
    struct mutex *mtx = (struct mutex*)handle;

    kfree(mtx);
};

acpi_cpu_flags acpi_os_acquire_lock(acpi_spinlock lockp)
{
    acpi_cpu_flags flags;
    spin_lock_irqsave(lockp, flags);
    return flags;
}


void acpi_os_release_lock(acpi_spinlock lockp, acpi_cpu_flags flags)
{
    spin_unlock_irqrestore(lockp, flags);
}


acpi_status acpi_os_signal(u32 function, void *info)
{
        switch (function) {
        case ACPI_SIGNAL_FATAL:
                printk(KERN_ERR PREFIX "Fatal opcode executed\n");
                break;
        case ACPI_SIGNAL_BREAKPOINT:
                /*
                 * AML Breakpoint
                 * ACPI spec. says to treat it as a NOP unless
                 * you are debugging.  So if/when we integrate
                 * AML debugger into the kernel debugger its
                 * hook will go here.  But until then it is
                 * not useful to print anything on breakpoints.
                 */
                break;
        default:
                break;
        }

        return AE_OK;
}

void acpi_os_sleep(u64 ms)
{
        msleep(ms);
}

void acpi_os_stall(u32 us)
{
        while (us) {
                u32 delay = 1000;

                if (delay > us)
                        delay = us;
                udelay(delay);
//                touch_nmi_watchdog();
                us -= delay;
        }
}

void msleep(unsigned int msecs)
{
    msecs /= 10;
    if(!msecs) msecs = 1;

     __asm__ __volatile__ (
     "call *__imp__Delay"
     ::"b" (msecs));
     __asm__ __volatile__ (
     "":::"ebx");

};

acpi_status acpi_os_execute(acpi_execute_type type,
                            acpi_osd_exec_callback function, void *context)
{

    return AE_OK;
}


u64 acpi_os_get_timer(void)
{
    u64 time_ns = ktime_to_ns(ktime_get());
    do_div(time_ns, 100);
    return time_ns;
}

ktime_t ktime_get(void)
{
    ktime_t t;

    t.tv64 = GetClockNs();

    return t;
}

void __delay(unsigned long loops)
{
        asm volatile(
                "test %0,%0\n"
                "jz 3f\n"
                "jmp 1f\n"

                ".align 16\n"
                "1: jmp 2f\n"

                ".align 16\n"
                "2: dec %0\n"
                " jnz 2b\n"
                "3: dec %0\n"

                : /* we don't need output */
                : "a" (loops)
        );
}


inline void __const_udelay(unsigned long xloops)
{
        int d0;

        xloops *= 4;
        asm("mull %%edx"
                : "=d" (xloops), "=&a" (d0)
                : "1" (xloops), ""
                (loops_per_jiffy * (HZ/4)));

        __delay(++xloops);
}

void __udelay(unsigned long usecs)
{
        __const_udelay(usecs * 0x000010c7); /* 2**32 / 1000000 (rounded up) */
}


#define acpi_rev_override       false

#define ACPI_MAX_OVERRIDE_LEN 100

static char acpi_os_name[ACPI_MAX_OVERRIDE_LEN];


acpi_status
acpi_os_predefined_override(const struct acpi_predefined_names *init_val,
                            char **new_val)
{
        if (!init_val || !new_val)
                return AE_BAD_PARAMETER;

        *new_val = NULL;
        if (!memcmp(init_val->name, "_OS_", 4) && strlen(acpi_os_name)) {
                printk(KERN_INFO PREFIX "Overriding _OS definition to '%s'\n",
                       acpi_os_name);
                *new_val = acpi_os_name;
        }

        if (!memcmp(init_val->name, "_REV", 4) && acpi_rev_override) {
                printk(KERN_INFO PREFIX "Overriding _REV return value to 5\n");
                *new_val = (char *)5;
        }

        return AE_OK;
}

