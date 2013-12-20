/*
 * exeption_handlers.c
 *
 *  Created on: 09 дек. 2013 г.
 *      Author: 17095
 */

#include <stdio.h>
#include <sys/stat.h>

//support prototypes
void hard_fault_handler_c(unsigned int * hardfault_args);
extern void __attribute__ ((weak)) Default_Handler();
//handlers
void NMI_Handler() {
	Default_Handler();
}
__attribute__((__interrupt__))

void HardFault_Handler() {
	asm("TST LR, #4");
	asm ("ITE EQ");
	asm ("MRSEQ R0, MSP ");
	asm (" MRSNE R0, PSP ");
	asm ("B hard_fault_handler_c");
}
__attribute__((__interrupt__))

//void HardFault_Handler() __attribute__ ((weak, alias("Default_Handler")));
void MemManage_Handler() {
	Default_Handler();
	//return;
}
__attribute__((__interrupt__))

void BusFault_Handler() {
	Default_Handler();
	//return;
}
__attribute__((__interrupt__))

void UsageFault_Handler() {
	Default_Handler();
	//return;
}
__attribute__((__interrupt__))

void SVC_Handler() {
	Default_Handler();
	//return;
}
__attribute__((__interrupt__))

void DebugMonitor_Handler() {
	Default_Handler();
	//return;
}
__attribute__((__interrupt__))

//And then you can extract the stacked registers in C:
// hard fault handler in C,
// with stack frame location as input parameter
void hard_fault_handler_c(unsigned int * hardfault_args) {
	unsigned int stacked_r0;
	unsigned int stacked_r1;
	unsigned int stacked_r2;
	unsigned int stacked_r3;
	unsigned int stacked_r12;
	unsigned int stacked_lr;
	unsigned int stacked_pc;
	unsigned int stacked_psr;

	stacked_r0 = ((unsigned long) hardfault_args[0]);
	stacked_r1 = ((unsigned long) hardfault_args[1]);
	stacked_r2 = ((unsigned long) hardfault_args[2]);
	stacked_r3 = ((unsigned long) hardfault_args[3]);

	stacked_r12 = ((unsigned long) hardfault_args[4]);
	stacked_lr = ((unsigned long) hardfault_args[5]);
	stacked_pc = ((unsigned long) hardfault_args[6]);
	stacked_psr = ((unsigned long) hardfault_args[7]);
/*
	printf("[Hard fault handler]\n");
	printf("R0 = %x\n", stacked_r0);
	printf("R1 = %x\n", stacked_r1);
	printf("R2 = %x\n", stacked_r2);
	printf("R3 = %x\n", stacked_r3);
	printf("R12 = %x\n", stacked_r12);
	printf("LR = %x\n", stacked_lr);
	printf("PC = %x\n", stacked_pc);
	printf("PSR = %x\n", stacked_psr);
	printf("BFAR = %x\n", (*((volatile unsigned long *) (0xE000ED38))));
	printf("CFSR = %x\n", (*((volatile unsigned long *) (0xE000ED28))));
	printf("HFSR = %x\n", (*((volatile unsigned long *) (0xE000ED2C))));
	printf("DFSR = %x\n", (*((volatile unsigned long *) (0xE000ED30))));
	printf("AFSR = %x\n", (*((volatile unsigned long *) (0xE000ED3C))));

	//exit(0); // terminate

	return;
	*/


	while(1);
}

