/** 
 * FILE: Exceptions_p1.c
 * 
 * COPYRIGHT (c) 2002-2012 Freescale Semiconductor, Inc. All rights reserved.
 *
 * DESCRIPTION: Setup of Core_1 IVPR to point to the EXCEPTION_HANDLERS_P1 memory area 
 *               defined in the linker command file.
 *               Default setup of the IVORxx registers.
 *               
 * Note: We use "_p0" suffix on functions and section names to reference the
 * Core_0, "_p1" for functions and section names using the Core_1.
 * 
 * VERSION: 1.1
 */


/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/

#include "Exceptions_p1.h" /* Implement functions from this file */

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/* Function Implementations                                                   */
/*----------------------------------------------------------------------------*/

#pragma push /* Save the current state */
/* Symbol EXCEPTION_HANDLERS_P1 is defined in the application linker command file (.lcf) 
   It is defined to the start of the code memory area used for the .__EXCEPTION_HANDLERS_P1 section. 
 */
/*lint -esym(752, EXCEPTION_HANDLERS_P1) */
__declspec (section ".__exception_handlers_p1") extern long EXCEPTION_HANDLERS_P1;  
#pragma force_active on

#pragma function_align 16 /* We use 16 bytes alignment for Exception handlers */
__declspec(interrupt)
__declspec (section ".__exception_handlers_p1")
void EXCEP_DefaultExceptionHandler_p1(void)
{

}
#pragma force_active off
#pragma pop

__asm void EXCEP_InitExceptionHandlers_p1(void)
{
	nofralloc
	/* Set the IVPR to the Exception Handlers address defined in the lcf file
	 * IVPR 0-15 bits Vector Base
	 * IVPR 16-31 bits ignored
	 * */
	lis     r0, EXCEPTION_HANDLERS_P1@h
	mtivpr  r0

	li		r0, EXCEPTION_HANDLERS_P1@l
	mtivor0 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0x10)@l
	mtivor1 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0x20)@l
	mtivor2 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0x30)@l
	mtivor3 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0x40)@l
	mtivor4 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0x50)@l
	mtivor5 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0x60)@l
	mtivor6 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0x70)@l
	mtivor7 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0x80)@l
	mtivor8 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0x90)@l
	mtivor9 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0xA0)@l
	mtivor10 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0xB0)@l
	mtivor11 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0xC0)@l
	mtivor12 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0xD0)@l
	mtivor13 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0xE0)@l
	mtivor14 r0

	li		r0, (EXCEPTION_HANDLERS_P1 + 0xF0)@l
	mtivor15 r0

	blr
}
#ifdef __cplusplus
}
#endif
