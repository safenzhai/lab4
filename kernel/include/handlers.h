/*
 * handlers.h: Defines constants used within handlers.c 
 * Authors: Sridhar Srinivasan <sridhar1@andrew.cmu.edu>
 *          Ramya Bolla <rbolla@andrew.cmu.edu>
 *          Vinay Prasad <vinayp1@andrew.cmu.edu>
 */
 
#ifndef HANDLERS_H
#define HANDLERS_H
#include <types.h>

/*
 * constants
 */
#define ACTUAL_PC_OFFSET 0x8
#define SWI_VECTOR_ADDR 0x8
#define IRQ_VECTOR_ADDR 0x18
#define LDR_INST_OFFSET 0x00000FFF
#define LDR_VALID_FORMAT 0xe59ff000
#define CUSTOM_HANDLER_INST1 0xe51ff004
#if 0
/*
 * signatures for syscall implementations
 */
ssize_t kread(int fd, void *buf, size_t count);
ssize_t kwrite(int fd, const void *buf, size_t count);
void kexit(int status);
unsigned long ktime(void);
void ksleep(unsigned long);
#endif
/*
 * prototypes
 */
int install_handler(unsigned int *vector_addr, void *handler_addr);
void s_handler(void);
//void i_handler(void);
//void init_irq_regs(void);
#if 0
void disable_intr(void);
#endif
#endif /* HANDLERS_H */
