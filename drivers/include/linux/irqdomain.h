#ifndef _LINUX_IRQDOMAIN_H
#define _LINUX_IRQDOMAIN_H
/*
 * KolibriOS has no irq domain layer.
 *
 * amdgpu builds a linear domain purely so that a *different* driver (the ACP
 * audio co-processor) can receive GPU interrupts by Linux irq number.  ACP is
 * not part of this port, so no mapping is ever created: adev->irq.virq[] stays
 * zero and amdgpu_irq_dispatch() always takes its direct-call branch.
 *
 * These declarations therefore exist to keep the domain setup compiling and
 * succeeding; the implementations in the driver's kos_stubs.c do nothing.
 */
#include <linux/types.h>
#include <linux/irq.h>

typedef unsigned long irq_hw_number_t;

struct irq_domain;
struct device_node;

struct irq_domain_ops {
	int  (*map)(struct irq_domain *d, unsigned int virq, irq_hw_number_t hw);
	void (*unmap)(struct irq_domain *d, unsigned int virq);
	int  (*xlate)(struct irq_domain *d, struct device_node *node,
		      const u32 *intspec, unsigned int intsize,
		      unsigned long *out_hwirq, unsigned int *out_type);
};

struct irq_domain *irq_domain_add_linear(struct device_node *of_node,
					 unsigned int size,
					 const struct irq_domain_ops *ops,
					 void *host_data);
void	 irq_domain_remove(struct irq_domain *domain);
unsigned int irq_create_mapping(struct irq_domain *domain, irq_hw_number_t hwirq);
unsigned int irq_find_mapping(struct irq_domain *domain, irq_hw_number_t hwirq);
int	 generic_handle_irq(unsigned int irq);
void	 irq_set_chip_and_handler(unsigned int irq, struct irq_chip *chip,
				  void (*handle)(void));

#endif /* _LINUX_IRQDOMAIN_H */
