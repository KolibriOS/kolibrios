#include <ddk.h>
#include <syscall.h>
#include <pci.h>

#define CPUID_VENDOR_LENGTH     3               /* 3 GPRs hold vendor ID */
#define CPUID_VENDOR_STR_LENGTH (CPUID_VENDOR_LENGTH * sizeof(uint32_t) + 1)
#define CPUID_BRAND_LENGTH      12              /* 12 GPRs hold vendor ID */
#define CPUID_BRAND_STR_LENGTH  (CPUID_BRAND_LENGTH * sizeof(uint32_t) + 1)

typedef union {
   unsigned char ch[48];
   uint32_t      uint[12];
   struct {
      uint32_t    fill1:24;      /* Bit 0 */
      uint32_t    l1_i_sz:8;
      uint32_t    fill2:24; 
      uint32_t    l1_d_sz:8;
      uint32_t    fill3:16; 
      uint32_t    l2_sz:16;
      uint32_t    fill4:18; 
      uint32_t    l3_sz:14;
      uint32_t    fill5[8];
   } amd;
} cpuid_cache_info_t;

/* Typedef for storing the CPUID Vendor String */
typedef union {
   /* Note: the extra byte in the char array is for '\0'. */
   char           char_array[CPUID_VENDOR_STR_LENGTH];
   uint32_t       uint32_array[CPUID_VENDOR_LENGTH];
} cpuid_vendor_string_t;

/* Typedef for storing the CPUID Brand String */
typedef union {
   /* Note: the extra byte in the char array is for '\0'. */
   char           char_array[CPUID_BRAND_STR_LENGTH];
   uint32_t       uint32_array[CPUID_BRAND_LENGTH];
} cpuid_brand_string_t;

/* Typedef for storing CPUID Version */
typedef union {
   uint32_t flat;
   struct {
      uint32_t    stepping:4;      /* Bit 0 */
      uint32_t    model:4;
      uint32_t    family:4;
      uint32_t    processorType:2;
      uint32_t    reserved1514:2;
      uint32_t    extendedModel:4;
      uint32_t    extendedFamily:8;
      uint32_t    reserved3128:4;  /* Bit 31 */
   } bits;      
} cpuid_version_t;

/* Typedef for storing CPUID Processor Information */
typedef union {
   uint32_t flat;
   struct {
      uint32_t    brandIndex:8;    /* Bit 0 */
      uint32_t    cflushLineSize:8;
      uint32_t    logicalProcessorCount:8;
      uint32_t    apicID:8;        /* Bit 31 */
   } bits;      
} cpuid_proc_info_t;

/* Typedef for storing CPUID Feature flags */
typedef union {
   uint32_t flat;
   struct {
      uint32_t    :1;           
   } bits;
} cpuid_custom_features;

/* Typedef for storing CPUID Feature flags */
typedef union {
   uint32_t       uint32_array[3];
   struct {
      uint32_t    fpu:1;           /* EDX feature flags, bit 0 */
      uint32_t    vme:1;
      uint32_t    de:1;
      uint32_t    pse:1;
      uint32_t    rdtsc:1;
      uint32_t    msr:1;
      uint32_t    pae:1;
      uint32_t    mce:1;
      uint32_t    cx8:1;
      uint32_t    apic:1;
      uint32_t    bit10:1;
      uint32_t    sep:1;
      uint32_t    mtrr:1;
      uint32_t    pge:1;
      uint32_t    mca:1;
      uint32_t    cmov:1;
      uint32_t    pat:1;
      uint32_t    pse36:1;
      uint32_t    psn:1;
      uint32_t    cflush:1;
      uint32_t    bit20:1;
      uint32_t    ds:1;
      uint32_t    acpi:1;
      uint32_t    mmx:1;
      uint32_t    fxsr:1;
      uint32_t    sse:1;
      uint32_t    sse2:1;
      uint32_t    ss:1;
      uint32_t    htt:1;
      uint32_t    tm:1;
      uint32_t    bit30:1;
      uint32_t    pbe:1;           /* EDX feature flags, bit 31 */
      uint32_t    sse3:1;          /* ECX feature flags, bit 0 */
      uint32_t    mulq:1;
      uint32_t    bit2:1;
      uint32_t    mon:1;
      uint32_t    dscpl:1;
      uint32_t    vmx:1;
     	uint32_t    smx:1;      	
     	uint32_t    eist:1;  
     	uint32_t    tm2:1;       		     		
      uint32_t    bits_9_31:23;
      uint32_t    bits0_28:29;     /* EDX extended feature flags, bit 0 */
      uint32_t    lm:1;		   /* Long Mode */
      uint32_t    bits_30_31:2;    /* EDX extended feature flags, bit 32 */
   } bits;
} cpuid_feature_flags_t;

/* An overall structure to cache all of the CPUID information */
struct cpu_ident {
	uint32_t max_cpuid;
	uint32_t max_xcpuid;
	uint32_t dts_pmp;
	cpuid_version_t vers;
	cpuid_proc_info_t info;
	cpuid_feature_flags_t fid;
	cpuid_vendor_string_t vend_id;
	cpuid_brand_string_t brand_id;
	cpuid_cache_info_t cache_info;
	cpuid_custom_features custom;
};

struct cpuid4_eax {
	uint32_t	ctype:5;
	uint32_t	level:3;
	uint32_t	is_self_initializing:1;
	uint32_t	is_fully_associative:1;
	uint32_t	reserved:4;
	uint32_t	num_threads_sharing:12;
	uint32_t	num_cores_on_die:6;
};

struct cpuid4_ebx {
	uint32_t	coherency_line_size:12;
	uint32_t	physical_line_partition:10;
	uint32_t	ways_of_associativity:10;
};

struct cpuid4_ecx {
	uint32_t	number_of_sets:32;
};

unsigned imc_type=0;
struct cpu_ident cpu_id;
bool temp_out_disable=false;

#define PCI_CONF_TYPE_NONE 0
#define PCI_CONF_TYPE_1    1
#define PCI_CONF_TYPE_2    2

extern struct cpu_ident cpu_id;

static unsigned char pci_conf_type = PCI_CONF_TYPE_NONE;

#define PCI_CONF1_ADDRESS(bus, dev, fn, reg) \
	(0x80000000 | (bus << 16) | (dev << 11) | (fn << 8) | (reg & ~3))

#define PCI_CONF2_ADDRESS(dev, reg)	(unsigned short)(0xC000 | (dev << 8) | reg)

#define PCI_CONF3_ADDRESS(bus, dev, fn, reg) \
	(0x80000000 | (((reg >> 8) & 0xF) << 24) | (bus << 16) | ((dev & 0x1F) << 11) | (fn << 8) | (reg & 0xFF))

int pci_conf_read(unsigned bus, unsigned dev, unsigned fn, unsigned reg, unsigned len, unsigned long *value)
{
	int result;

	if (!value || (bus > 255) || (dev > 31) || (fn > 7) || (reg > 255 && pci_conf_type != PCI_CONF_TYPE_1))
		return -1;

	result = -1;
	switch(pci_conf_type) {
	case PCI_CONF_TYPE_1:
		if(reg < 256){
			outl(PCI_CONF1_ADDRESS(bus, dev, fn, reg), 0xCF8);
		}else{
			outl(PCI_CONF3_ADDRESS(bus, dev, fn, reg), 0xCF8);		
		}
		switch(len) {
		case 1:  *value = inb(0xCFC + (reg & 3)); result = 0; break;
		case 2:  *value = inw(0xCFC + (reg & 2)); result = 0; break;
		case 4:  *value = inl(0xCFC); result = 0; break;
		}
		break;
	case PCI_CONF_TYPE_2:
		outb(0xF0 | (fn << 1), 0xCF8);
		outb(bus, 0xCFA);

		switch(len) {
		case 1:  *value = inb(PCI_CONF2_ADDRESS(dev, reg)); result = 0; break;
		case 2:  *value = inw(PCI_CONF2_ADDRESS(dev, reg)); result = 0; break;
		case 4:  *value = inl(PCI_CONF2_ADDRESS(dev, reg)); result = 0; break;
		}
		outb(0, 0xCF8);
		break;
	}
	return result;
}

void detect_imc(void)
{
	// Check AMD IMC
	if(cpu_id.vend_id.char_array[0] == 'A' && cpu_id.vers.bits.family == 0xF) 
		{
			printk("extended family = %x\n", cpu_id.vers.bits.extendedFamily);
            switch(cpu_id.vers.bits.extendedFamily)
					{
						case 0x0:
							imc_type = 0x0100; // Old K8
							break;
						case 0x1:
						case 0x2:
							imc_type = 0x0101; // K10 (Family 10h & 11h)
							break;
						case 0x3:
							imc_type = 0x0102; // A-Series APU (Family 12h)
							break;
						case 0x5:
							imc_type = 0x0103; // C- / E- / Z- Series APU (Family 14h)
							break;	
						case 0x6:
							imc_type = 0x0104; // FX Series (Family 15h)
							break;								
						case 0x7:
							imc_type = 0x0105; // Kabini & related (Family 16h)
							break;			
					}	
			return;
		}
					
	// Check Intel IMC	
	if(cpu_id.vend_id.char_array[0] == 'G' && cpu_id.vers.bits.family == 6 && cpu_id.vers.bits.extendedModel) 
		{					
			switch(cpu_id.vers.bits.model)
			{
				case 0x5:
					if(cpu_id.vers.bits.extendedModel == 2) { imc_type = 0x0003; } // Core i3/i5 1st Gen 45 nm (NHM)
					if(cpu_id.vers.bits.extendedModel == 3) { temp_out_disable=true; } // Atom Clover Trail
					if(cpu_id.vers.bits.extendedModel == 4) { imc_type = 0x0007; } // HSW-ULT
					break;
				case 0x6:
					if(cpu_id.vers.bits.extendedModel == 3) { 
						imc_type = 0x0009;  // Atom Cedar Trail
						temp_out_disable=true;
						//v->fail_safe |= 4; // Disable Core temp
					}
					break;
				case 0xA:
					switch(cpu_id.vers.bits.extendedModel)
					{
						case 0x1:
							imc_type = 0x0001; // Core i7 1st Gen 45 nm (NHME)
							break;
						case 0x2:
							imc_type = 0x0004; // Core 2nd Gen (SNB)
							break;	
						case 0x3:
							imc_type = 0x0006; // Core 3nd Gen (IVB)						
							break;
					}
					break;
				case 0xC:
					switch(cpu_id.vers.bits.extendedModel)
					{
						case 0x1:
							if(cpu_id.vers.bits.stepping > 9) { imc_type = 0x0008; } // Atom PineView	
							//v->fail_safe |= 4; // Disable Core temp
							temp_out_disable=true;
							break;	
						case 0x2:
							imc_type = 0x0002; // Core i7 1st Gen 32 nm (WMR)	
							break;	
						case 0x3:
							imc_type = 0x0007; // Core 4nd Gen (HSW)						
							break;
					}
					break;			
				case 0xD:
					imc_type = 0x0005; // SNB-E
					break;				
				case 0xE:
					imc_type = 0x0001; // Core i7 1st Gen 45 nm (NHM)
					break;				
			}
		
		//if(imc_type) { tsc_invariable = 1; }
		return;
		}
}

static int pci_check_direct(void)
{
	unsigned char tmpCFB;
	unsigned int  tmpCF8;
	
	if (cpu_id.vend_id.char_array[0] == 'A' && cpu_id.vers.bits.family == 0xF) {
			pci_conf_type = PCI_CONF_TYPE_1;
			return 0;		
	} else {
			/* Check if configuration type 1 works. */
			pci_conf_type = PCI_CONF_TYPE_1;
			tmpCFB = inb(0xCFB);
			outb(0x01, 0xCFB);
			tmpCF8 = inl(0xCF8);
			outl(0x80000000, 0xCF8);
			if ((inl(0xCF8) == 0x80000000) && (pci_sanity_check() == 0)) {
				outl(tmpCF8, 0xCF8);
				outb(tmpCFB, 0xCFB);
				return 0;
			}
			outl(tmpCF8, 0xCF8);
		
			/* Check if configuration type 2 works. */
			
			pci_conf_type = PCI_CONF_TYPE_2;
			outb(0x00, 0xCFB);
			outb(0x00, 0xCF8);
			outb(0x00, 0xCFA);
			if (inb(0xCF8) == 0x00 && inb(0xCFA) == 0x00 && (pci_sanity_check() == 0)) {
				outb(tmpCFB, 0xCFB);
				return 0;
				
	}

	outb(tmpCFB, 0xCFB);

	/* Nothing worked return an error */
	pci_conf_type = PCI_CONF_TYPE_NONE;
	return -1;
	
	}
}


#define PCI_BASE_CLASS_BRIDGE		0x06
#define PCI_CLASS_BRIDGE_HOST		0x0600
#define PCI_CLASS_DEVICE        0x0a    /* Device class */

int pci_sanity_check(void)
{
	unsigned long value;
	int result;
	/* Do a trivial check to make certain we can see a host bridge.
	 * There are reportedly some buggy chipsets from intel and
	 * compaq where this test does not work, I will worry about
	 * that when we support them.
	 */
	result = pci_conf_read(0, 0, 0, PCI_CLASS_DEVICE, 2, &value);
	if (result == 0) {
		result = -1;
		if (value == PCI_CLASS_BRIDGE_HOST) {
			result = 0;
		}
	}
	return result;
}

int pci_init(void)
{
	int result;
	/* For now just make certain we can directly
	 * use the pci functions.
	 */
	result = pci_check_direct();
	return result;
}


void get_cpuid()
{
	unsigned int *v, dummy[3];
	char *p, *q;

	/* Get max std cpuid & vendor ID */
	cpuid(0x0, &cpu_id.max_cpuid, &cpu_id.vend_id.uint32_array[0],
	    &cpu_id.vend_id.uint32_array[2], &cpu_id.vend_id.uint32_array[1]);
	cpu_id.vend_id.char_array[11] = 0;

	/* Get processor family information & feature flags */
	if (cpu_id.max_cpuid >= 1) {
	    cpuid(0x00000001, &cpu_id.vers.flat, &cpu_id.info.flat,
		&cpu_id.fid.uint32_array[1], &cpu_id.fid.uint32_array[0]);
	}

	/* Get the digital thermal sensor & power management status bits */
	if(cpu_id.max_cpuid >= 6)	{
		cpuid(0x00000006, &cpu_id.dts_pmp, &dummy[0], &dummy[1], &dummy[2]);
	}
	
	/* Get the max extended cpuid */
	cpuid(0x80000000, &cpu_id.max_xcpuid, &dummy[0], &dummy[1], &dummy[2]);

	/* Get extended feature flags, only save EDX */
	if (cpu_id.max_xcpuid >= 0x80000001) {
	    cpuid(0x80000001, &dummy[0], &dummy[1],
		&dummy[2], &cpu_id.fid.uint32_array[2]);
	}

	/* Get the brand ID */
	if (cpu_id.max_xcpuid >= 0x80000004) {
	    v = (unsigned int *)&cpu_id.brand_id;
	    cpuid(0x80000002, &v[0], &v[1], &v[2], &v[3]);
	    cpuid(0x80000003, &v[4], &v[5], &v[6], &v[7]);
	    cpuid(0x80000004, &v[8], &v[9], &v[10], &v[11]);
	    cpu_id.brand_id.char_array[47] = 0;
	}
        /*
         * Intel chips right-justify this string for some dumb reason;
         * undo that brain damage:
         */
        p = q = &cpu_id.brand_id.char_array[0];
        while (*p == ' ')
                p++;
        if (p != q) {
                while (*p)
                        *q++ = *p++;
                while (q <= &cpu_id.brand_id.char_array[48])
                        *q++ = '\0';    /* Zero-pad the rest */
	}

	/* Get cache information */
	switch(cpu_id.vend_id.char_array[0]) {
        case 'A':
            /* AMD Processors */
	    /* The cache information is only in ecx and edx so only save
	     * those registers */
	    if (cpu_id.max_xcpuid >= 0x80000005) {
		cpuid(0x80000005, &dummy[0], &dummy[1],
		    &cpu_id.cache_info.uint[0], &cpu_id.cache_info.uint[1]);
	    }
	    if (cpu_id.max_xcpuid >= 0x80000006) {
		cpuid(0x80000006, &dummy[0], &dummy[1],
		    &cpu_id.cache_info.uint[2], &cpu_id.cache_info.uint[3]);
	    }
	    break;
	case 'G':
                /* Intel Processors, Need to do this in init.c */
	    break;
	}

	/* Turn off mon bit since monitor based spin wait may not be reliable */
	cpu_id.fid.bits.mon = 0;

}

void coretemp(void) 
{
	unsigned int msrl, msrh;
	unsigned int tjunc, tabs, tnow;
	unsigned long rtcr;
	long amd_raw_temp=524322;
	
	// Only enable coretemp if IMC is known
	if(imc_type == 0) { return; }
	
	tnow = 0;
	
	// Intel  CPU
	if(cpu_id.vend_id.char_array[0] == 'G' && cpu_id.max_cpuid >= 6)
	{
		if(cpu_id.dts_pmp & 1){
			rdmsr(MSR_IA32_THERM_STATUS, msrl, msrh);
			tabs = ((msrl >> 16) & 0x7F);
			rdmsr(MSR_IA32_TEMPERATURE_TARGET, msrl, msrh);
			tjunc = ((msrl >> 16) & 0x7F);
			if(tjunc < 50 || tjunc > 125) { tjunc = 90; } // assume Tjunc = 90°C if boggus value received.
			tnow = tjunc - tabs;		
			//dprint(LINE_CPU+1, 30, v->check_temp, 3, 0);	
			printk("temp=%d\n", tnow);
		}
		return;
	}
	
	// AMD CPU
	if(cpu_id.vend_id.char_array[0] == 'A' && cpu_id.vers.bits.extendedFamily > 0)
	{
		pci_conf_read(0, 24, 3, 0xA4, 4, &rtcr);
		amd_raw_temp = ((rtcr >> 21) & 0x7FF);
        printk("temp=%d\n", amd_raw_temp/8);
	}	
				
} 

unsigned drvEntry(int action, char *cmdline){
    get_cpuid();
    pci_init();
    detect_imc();
    if(!temp_out_disable){
        coretemp();
    }
}
