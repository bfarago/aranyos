/*
 * FILE : MPC5643L_DPM_Startup.c
 *	
 * COPYRIGHT (c) 2007-2012 Freescale Semiconductor, Inc. All rights reserved.
 * 	
 * DESCRIPTION: This file contains the entry point __startup for the MCU in Decoupled parallel mode.
 * 
 * VERSION: 1.1 
 */

#ifdef __cplusplus
extern "C" {
#endif

__declspec(section ".init") extern void __startup(int argc, char **argv, char **envp);				/* primary entry point */
extern asm void __start(register int argc, register char **argv, register char **envp);

#ifdef __cplusplus
}
#endif


asm void __startup(register int argc, register char **argv, register char **envp)
{
	nofralloc							/* explicitly no stack */
										/* frame allocation */
	// call standard application initialization
	bl __start
}
