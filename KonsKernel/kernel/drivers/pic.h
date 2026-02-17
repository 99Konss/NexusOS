// kernel/drivers/pic.h
#ifndef KERNEL_DRIVERS_PIC_H
#define KERNEL_DRIVERS_PIC_H

void pic_remap(int offset1, int offset2);
void pic_send_eoi(unsigned char irq);

#endif
