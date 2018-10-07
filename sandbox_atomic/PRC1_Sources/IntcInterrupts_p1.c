/** 
 * FILE: IntcInterrupts_p1.c
 * 
 * COPYRIGHT (c) 2002-2012 Freescale Semiconductor, Inc. All rights reserved.
 * 
 * DESCRIPTION: Contains the implementations of the generic interrupt 
 * 	controller handling routines for the e200z4 MCU capable of LS/DP.
 * 
 * Note: We use "_p0" suffix on functions and section names to reference the
 * Core_0, "_p1" for functions and section names using the Core_1.
 * 
 * The following code setup the INTC module for Software Vector Mode.
 * To enable the Harware Vector Mode please consult the Qorivva cookbook AN2865.
 *  
 * VERSION: 1.2
*/

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* By default we do not use nested interrupts, interrupts are handled in C.  */
/* In case you want to use nested interrupts, set INTC_NESTED_INTERRUPT.     */
/* In this case make sure all the needed registers are saved in the prolog   */
/* and epilog of asm void INTC_INTCInterruptHandler(void)                    */
/*---------------------------------------------------------------------------*/

#ifndef INTC_NESTED_INTERRUPT
#define INTC_NESTED_INTERRUPT 0
#endif

/*---------------------------------------------------------------------------*/
/* Includes                                                                  */
/*---------------------------------------------------------------------------*/

#include "MPC5643L.h"        /* MCU platform development header       */

#include "IntcInterrupts_p1.h"     /* Implement functions from this file */

/*---------------------------------------------------------------------------*/
/* Inline Assembler Defines                                                  */
/*---------------------------------------------------------------------------*/

/* This macro allows to use C defined address with the inline assembler */
#define MAKE_HLI_ADDRESS(hli_name, c_expr) /*lint -e753 */enum { hli_name=/*lint -e30*/((int)(c_expr)) /*lint -esym(749, hli_name) */ };

/* Address of the IACKR Interrupt Controller register. */
MAKE_HLI_ADDRESS(INTC_IACKR, &INTC_1.IACKR_PRC0.R)

/* Address of the EOIR End-of-Interrupt Register register. */
MAKE_HLI_ADDRESS(INTC_EOIR, &INTC_1.EOIR_PRC0.R)

/* INTC.MCR -- caution, VTES and HVEN for both cores are in this register */
MAKE_HLI_ADDRESS(INTC_MCR, &INTC_1.MCR.R)

/*----------------------------------------------------------------------------*/
/* Function Implementations                                                   */
/*----------------------------------------------------------------------------*/

/* This is the interrupt service routine table used in software mode.
   The INTC supports 4 bytes or 8 byte entries, we will use 4 byte entries.
   The 4/8 bytes mode is configured in INTC_InitINTCInterrupts.
*/
#define INTC_INTERRUPTS_REQUEST_VECTOR_TABLE_SIZE_P1 (255 * 4)

#pragma push /* Save the current state */
#pragma section data_type ".__initialized_intc_handlertable" ".__uninitialized_intc_handlertable_p1"
/* The INTC vector table will be placed in RAM. 
   We will use the ".__uninitialized_intc_handlertable" name to do the 
   placement in the Linker Command File (.lcf) to avoid the initialization at 
   startup time.  This will decrease the code size, but the table won't be 
   initialized to zero.
*/ 
INTCInterruptFn INTCInterruptsHandlerTable_p1[INTC_INTERRUPTS_REQUEST_VECTOR_TABLE_SIZE_P1];
#pragma pop

#pragma push             /* save the current state */
#pragma force_active on  /* make sure this function remains */
#pragma function_align 16 /* We use 16 bytes alignment for Exception handlers */
/** Handle the interrupt source by jumping to the ISR branch table (IACKR) */

#if INTC_NESTED_INTERRUPT == 0

/* The common interrupt exception handler called in software mode.
   The INTC_IACKR provides a value that can be used to load the address
   of an ISR from a vector table. The vector table can be composed of
   addresses of the ISRs specific to their respective interrupt vectors.
   In this case the vector table is INTCInterruptsHandlerTable.
 */
__declspec(section ".__exception_handlers_p1")
__declspec(interrupt)
void INTC_INTCInterruptHandler_p1(void)
{
    INTCInterruptFn *handlerFn = (INTCInterruptFn*)(*(unsigned int*)&INTC_1.IACKR_PRC0.R);

    (**handlerFn)();

    INTC_1.EOIR_PRC0.R = 0;
}

#else

__declspec(interrupt)
__declspec(section ".__exception_handlers_p1")
__asm void INTC_INTCInterruptHandler_p1(void)
{
nofralloc
prolog:
    stwu    r1, -0x50 (r1)      /* Create stack frame */
    stw r0,  0x24 (r1)          /* Store r0 working register  */

    /* Save SRR0 and SRR1 */
    mfsrr1  r0                  /* Store SRR1 (before enabling EE) */
    stw     r0,  0x10 (r1)
    mfsrr0  r0                  /* Store SRR0 (before enabling EE) */
    stw     r0,  0x0C (r1)

    /* Clear request to processor; r3 contains the address of the ISR */
    stw     r3,  0x28 (r1)      /* Store r3 to work with */
    lis     r3, INTC_IACKR@ha    /* Read pointer into ISR Vector Table */
    lwz     r3, INTC_IACKR@l(r3) /* Load INTC_IACKR, which clears request to processor */
    lwz     r3, 0x0(r3)         /* Read ISR address from ISR Vector Table using pointer  */

    /* Enable processor recognition of interrupts */
    wrteei  1                   /* Set MSR[EE]=1  */

    /* Save rest of context required by EABI */
    stw     r12, 0x4C (r1)      /* Store r12 */
    stw     r11, 0x48 (r1)      /* Store r11 */
    stw     r10, 0x44 (r1)      /* Store r10 */
    stw     r9,  0x40 (r1)      /* Store r9 */
    stw     r8,  0x3C (r1)      /* Store r8 */
    stw     r7,  0x38 (r1)      /* Store r7 */
    stw     r6,  0x34 (r1)      /* Store r6 */
    stw     r5,  0x30 (r1)      /* Store r5 */
    stw     r4,  0x2C (r1)      /* Store r4 */
    mfcr    r0                  /* Store CR */
    stw     r0,  0x20 (r1)
    mfxer   r0                  /* Store XER */
    stw     r0,  0x1C (r1)
    mfctr   r0                  /* Store CTR */
    stw     r0,  0x18 (r1)
    mflr    r0                  /* Store LR */
    stw     r0,  0x14 (r1)

    /* Branch to ISR handler address from SW vector table */
    mtlr    r3                 /* Store ISR address to LR */
    blrl                       /* Branch to ISR, but return here */

epilog:
    /* Restore context required by EABI (except working registers) */
    lwz     r0,  0x14 (r1)      /* Restore LR */
    mtlr    r0
    lwz     r0,  0x18 (r1)      /* Restore CTR */
    mtctr   r0
    lwz     r0,  0x1C (r1)      /* Restore XER */
    mtxer   r0
    lwz     r0,  0x20 (r1)      /* Restore CR */
    mtcrf   0xff, r0
    lwz     r5,  0x30 (r1)      /* Restore r5 */
    lwz     r6,  0x34 (r1)      /* Restore r6 */
    lwz     r7,  0x38 (r1)      /* Restore r7 */
    lwz     r8,  0x3C (r1)      /* Restore r8 */
    lwz     r9,  0x40 (r1)      /* Restore r9 */
    lwz     r10, 0x44 (r1)      /* Restore r10 */
    lwz     r11, 0x48 (r1)      /* Restore r11 */
    lwz     r12, 0x4C (r1)      /* Restore r12 */

    /* Disable processor recognition of interrupts */
    wrteei  0

    /* Ensure interrupt flag has finished clearing */
    mbar    0

    /* Write 0 to INTC_EOIR, informing INTC to lower priority */
    li      r3, 0
    lis     r4, INTC_EOIR@ha    /* Load upper half of INTC_EOIR address */
    stw     r3, INTC_EOIR@l(r4) /* Write 0 to INTC_EOIR */

    /* Restore Working Registers */
    lwz     r3,  0x28 (r1)      /* Restore r3 */
    lwz     r4,  0x2C (r1)      /* Restore r4 */

    /* Retrieve SRR0 and SRR1 */
    lwz     r0,  0x0C (r1)      /* Restore SRR0 */
    mtsrr0  r0
    lwz     r0,  0x10 (r1)      /* Restore SRR1 */
    mtsrr1  r0

    /* Restore Other Working Registers */
    lwz     r0,  0x24 (r1)      /* Restore r0 */

    /* Restore space on stack */
    addi    r1, r1, 0x50

    /* End of Interrupt */
    rfi
}

#endif

#pragma force_active off
#pragma pop

/**
 * This function can be used to install an interrupt handler for a given
 * interrupt vector. It will also set the Priority Status Register for the
 * source to the one given
 */
void INTC_InstallINTCInterruptHandler_p1(INTCInterruptFn handlerFn, unsigned short vectorNum,
                                      unsigned char psrPriority)
{
    /* Set the function pointer in the ISR Handler table */
    INTCInterruptsHandlerTable_p1[vectorNum] = handlerFn;
    /* Set the PSR Priority */
    INTC_1.PSR[vectorNum].B.PRI = psrPriority; 
}

/**
 * This function will setup INTC for Software Vector Mode and
 * the Vector Table Base Address to INTCInterruptsHandlerTable_p1.
 * This function can be used from user_init() (no stack frame, no memory access).
 */
__asm void INTC_InitINTCInterrupts_p1( void )
{
nofralloc
    /* Set the location of the ISR Handler Table in INTC IACKR Register */
    lis r0, INTCInterruptsHandlerTable_p1@h
    ori r0, r0, INTCInterruptsHandlerTable_p1@l
    lis r3,INTC_IACKR@ha
    stw r0, INTC_IACKR@l(r3)

    /* Set INTC.MCR.B.HVEN_PRC1 = 0 for SW vector mode               */
    /* and INTC.MCR.B.VTES_PRC1 = 0 for 4-byte branch table entries. */
    lis r3, INTC_MCR@ha
    lwz r0, INTC_MCR@l(r3)

    /* clear only the flag for core 0*/
    andi. r0, r0, 0xFF00
    stw r0, INTC_MCR@l(r3)

    /* Enable external interrupts by setting MSR[EE] = 1. */
    wrteei 1

    blr
}

#ifdef __cplusplus
}
#endif
