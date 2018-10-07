/*
 * FILE: IntcInterrupts_p1.h
 *
 * DESCRIPTION: Contains defines for utilizing the Interrupt Controller in 
 * the e200z4 MCU. 
 * 
 * Note: We use "_p0" suffix on functions and section names to reference the
 * Core_0, "_p1" for functions and section names using the Core_1.
 * 
 * VERSION: 1.2 
 */

#ifndef _INTCINTERRUPTS_P1_H_
#define _INTCINTERRUPTS_P1_H_

/*---------------------------------------------------------------------------*/
/* Types                                                                     */
/*---------------------------------------------------------------------------*/

/** All interrupt handlers should be of this type */
typedef void(*INTCInterruptFn)(void);

/*---------------------------------------------------------------------------*/
/* Function declarations                                                     */
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

#pragma section RX ".__exception_handlers_p1"

/**
 * This function will setup the PowerPC Zen core to jump to an Interrupt 
 * Service Routine.
 * This function can be used from user_init() (no stack frame, no memory 
 * access).
 */
__declspec(section ".__exception_handlers_p1")
__asm void INTC_InitINTCInterrupts_p1(void);

/**
 * This function can be used to install an interrupt handler for a given
 * interrupt vector. It will also set the Priority Status Register for the
 * source to the one given.
 * parameter handlerFn: The function to call when the interrupt occurs.
 * parameter vectoryNum: The number of the INTC Interrupt Request Source we 
 * wish to install the handler for.
 * parameter psrPriority: The priority to set in the Interrupt Controller 
 * Priority Select Register.
 */
__declspec(section ".__exception_handlers_p1")
void INTC_InstallINTCInterruptHandler_p1(INTCInterruptFn handlerFn, 
                                      unsigned short vectorNum,
                                      unsigned char psrPriority);

/**
 * This function is used to Handle the interrupt source by jumping to the ISR
 * branch table (IACKR)
 */
__declspec (section ".__exception_handlers_p1")
void INTC_INTCInterruptHandler_p1(void);

#ifdef __cplusplus
}
#endif

#endif

