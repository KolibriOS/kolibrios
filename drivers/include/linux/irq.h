#ifndef _LINUX_IRQ_H
#define _LINUX_IRQ_H
/*
 * KolibriOS has no irq_domain layer.  amdgpu_irq.c is patched to dispatch IH
 * ring entries straight to the registered amdgpu_irq_src, so only the types it
 * still mentions need to exist.
 */
#include <linux/types.h>

struct irq_data {
	unsigned int	 irq;
	unsigned long	 hwirq;
	void		*chip_data;
};

struct irq_chip {
	const char	*name;
	void		(*irq_mask)(struct irq_data *data);
	void		(*irq_unmask)(struct irq_data *data);
};

static inline void *irq_data_get_irq_chip_data(struct irq_data *d)
{
	return d ? d->chip_data : NULL;
}
static inline void handle_simple_irq(void) { }
#endif
