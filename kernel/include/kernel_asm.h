/*
 * kernel_asm.h: Defines constants used within the kernel_asm file 
 * Authors: Sridhar Srinivasan <sridhar1@andrew.cmu.edu>
 *          Ramya Bolla <rbolla@andrew.cmu.edu>
 *          Vinay Prasad <vinayp1@andrew.cmu.edu>
 */

#ifndef KERNEL__ASM_H
#define KERNEL__ASM_H

/*
 * stack offsets
 */
#define R8_OFFSET 60
#define SPSR_STACK_OFFSET 56
#define WORD_OFFSET 4
#define TWO_WORD_OFFSET 8
/*
 * masks
 */
//#define IRQ_MASK 0x80
#define SWI_NUM_MASK 0xff000000
#define CPSR_USER_MODE 0x10
//#define CPSR_IRQ_MODE 0x12
#define CPSR_SVC_MODE 0x13
#define CPSR_LAUNCH_APP_MASK 0x9F
#define CPSR_MODE_MASK 0x1F
#if 0
/*
 * values
 */
#define USER_APP_START 0xa0000000
#endif
#endif /* KERNEL_ASM_H */
