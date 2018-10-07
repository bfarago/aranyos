/**
* FILE:  __start_p1.c
* 
* COPYRIGHT (c) 2002-2012 Freescale Semiconductor, Inc. All rights reserved.
*  
* DESCRIPTION: A minimal startup function for Core_1 of a e200z4 family processor.
*
* This function initializes the stack to the values defined in the LCF file and
* uses the linker defined _SDA_BASE_ and _SDA2_BASE_ symbols to initialize
* r2 and r13 per (EABI). At the end it calls main_p1().
*
* Note: We use "_p0" suffix on functions and section names to reference the
* Core_0, "_p1" for functions and section names using the Core_1.
*
* COPYRIGHT (c) 2002-2012 Freescale Semiconductor, Inc. All rights reserved.
*
* Rev 1.2 - init core_1 MMU
*
* VERSION: 1.2
*/

#include <__ppc_eabi_init.h>
#include <__ppc_eabi_linker.h> /* linker-generated symbol declarations */

#include "Exceptions_p1.h"      /* core_1 IVPR and default exception handlers setup */
#include "IntcInterrupts_p1.h" /* core_1  INTC Interrupts Requests configuration */

#include "MPC5643L_HWInit.h"        /* MCU platform development HWInit header */

/* These symbols are defined in the Linker configuration file, but are not in */
/* __ppc_eabi_linker.h, so we'll provide the declarations here.  Note that    */
/* _heap_*_p1 definitions are currently not being used.                       */ 
__declspec(section ".init") extern char    _stack_addr_p1[];
__declspec(section ".init") extern char _stack_end_p1[];
__declspec(section ".init") extern char _heap_addr_p1[];
__declspec(section ".init") extern char _heap_end_p1[];

extern void main_p1();

#ifdef __cplusplus
extern "C" {
#endif

/*
 Note: We need to place this method in .init memory space since the core
 may need to access data/code from this space, eg. core MMU initialization code.
 This should happen for ROM targets, when core_1 is brought out of reset and
 the MMU is configured with a single 4K page. This space should allow the core to
 run the start up code.
 For internal RAM target it is assumed that the MMU it is already initialized
 by the debug tool.
 */
#pragma push
#if (ROM_VERSION == 1)
#pragma section code_type ".init"
#else
#pragma section RX ".text_p1"
#pragma section code_type ".text_p1"
#endif

#pragma function_align 8 /* need to align for correct VLE bit in SSCM_DPMBOOT when booting second core */
#pragma force_active on

/* Core_1 entry point. */
asm extern void __start_p1(void);

/*------------------------------------------------------------------*/
/* Entry point for core_1.                                          */
/*------------------------------------------------------------------*/
asm extern void __start_p1(void)
{
    nofralloc
    mflr r26
    /* use bctrl to call __initMMU_p1 since VLE/BOOKE APU branch
      bxx target field is up to 24-bit wide
    */
    lis r5, __initMMU_p1@h
    ori r5, r5, __initMMU_p1@l
    mtctr r5
    bctrl
    mtlr r26;

    /* initialize exception handlers for core 1*/
    mflr r26
    bl EXCEP_InitExceptionHandlers_p1
    mtlr r26

    /* Set up INTC Interrupts Requests handling */
    mflr r26
    bl INTC_InitINTCInterrupts_p1 
    mtlr r26

    /* Initialize stack pointer                */
    lis    r1, _stack_addr_p1@ha
    /* Emulate a calling function's frame, for correct LR save in main_p1 */
    addi    r1, r1, (_stack_addr_p1@l) - 16

    /* Note that we are assuming small data is shared between the cores */
    /* Initialize small data area pointers (EABI)                       */
    lis   r2, _SDA2_BASE_@ha
    addi  r2, r2, _SDA2_BASE_@l

    lis   r13, _SDA_BASE_@ha
    addi  r13, r13, _SDA_BASE_@l

    /* now call main p1 program */
    lis r5, main_p1@h
    ori r5, r5, main_p1@l
    mtctr r5
    bctrl

    /* if main returns, just hang here */
here:   b here

    blr
}

#pragma pop

#ifdef __cplusplus
}
#endif

