// kernel/memory/isr.h
#ifndef KERNEL_MEMORY_ISR_H
#define KERNEL_MEMORY_ISR_H

#include "../kernel.h"  // für struct regs

// Interrupt Handler
void isr_handler(struct regs *r);
void irq_handler(struct regs *r);

#endif
