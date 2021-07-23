#include <ddk.h>
#include <syscall.h>
#include <linux/pci.h>
#include <asm/processor.h>

void cpu_detect(struct cpuinfo_x86 *c)
{
    static u32 eax, dummy;    
    cpuid(1, &eax, &dummy, &dummy, (int *) &c->x86_capability);
	c->x86 = (eax >> 8) & 0xf;
	c->x86_model = (eax >> 4) & 0xf;
    
    if (c->x86 == 0xf){
	    c->x86 += (eax >> 20) & 0xff;
    }
	if (c->x86 >= 0x6){
	    c->x86_model += ((eax >> 16) & 0xf) << 4;
    }
	c->x86_mask = eax & 0xf;
}
