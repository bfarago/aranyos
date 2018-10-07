/*
 * FILE : MPC5643L_HWInit.c
 *    
 * COPYRIGHT (c) 2007-2012 Freescale Semiconductor, Inc. All rights reserved.
 *     
 * DESCRIPTION: This file contains a basic MPC5643L derivative initializations.
 *  This includes setting up the MMU, SRAM (LockStep or Decoupled), WatchDog and
 *  clearing the FCCU fault flags.
 *
 *  For more on Qorivva MCUs module initializations please consult the Qorivva cookbook AN2865.  
 *
 * Rev. 1.2 - clear fault flags and code cleanup; create MMU entries for basic modules.
 * Rev. 1.3 - fix MMU table entry for FlexCAN module access
 *  
 * VERSION: 1.3
 */

/*--------------------------------------------------------------*/
/* Includes                                                     */
/*--------------------------------------------------------------*/

#include "MPC5643L.h"       /* MPC55xx platform development header            */
#include "MPC5643L_HWInit.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------*/
/* Helper function declarations                                      */
/*-------------------------------------------------------------------*/

/* All these functions must be located in the initial 4KB memory window (.init) 
    and implemented "nofralloc" so as to not use the stack
*/

/*-------------------------------------------------------*/
/* Initialize the needed MMU Table entries. Common init. */        
/* function called by both core start code.                 */
/*-------------------------------------------------------*/
__declspec(section ".init") __asm void __initMMU(void);

/* Write one MMU Table Entry */
__declspec(section ".init") __asm void WriteMMUTableEntry( void );

/* Write one MMU Table Entry with context synchronization */
__declspec(section ".init") __asm void WriteMMUTableEntryS( void );

/*----------------------------------------------------------------------------*/
/* Function implementations                                                   */
/*----------------------------------------------------------------------------*/

/** Non-critical fault key */
#define FCCU_NCFK_KEY 0xAB3498FE

/** Critical fault key */
#define FCCU_CFK_KEY  0x618B7A50

MAKE_HLI_COMPATIBLE(FCCU_CF_S0, &FCCU.CF_S[0])
MAKE_HLI_COMPATIBLE(FCCU_NCF_S0, &FCCU.NCF_S[0])
/*------------------------------------------------------------------*/
/* FUNCTION     : __clearFaultFlags                                 */
/* PURPOSE      : Clears FCCU fault flags.                          */
/* SEQUENCE:    - clears critical and non-critical fault flags      */
/*                - put system to DRUN mode                         */
/*------------------------------------------------------------------*/
extern unsigned long L2SRAM_LOCATION_P1;
void __clearFaultFlags(void)
{
    if(RGM.FES.B.F_FCCU_SAFE /*|| RGM.FES.B.F_FCCU_HARD*/)
    {
        /* New mode requested other than RESET/SAFE while SAFE event is pending */
        ME.IMTS.R = 0x00000001;
        /* clear critical faults*/
        /* loop & clear FCCU.CF_Sx, x=0..3 */
        asm ("li r4, -1");/* r4 -> 0xFFFFFFFF: clearing N/CF flags requires '1' write.*/
        asm ("lis r3, FCCU_CF_S0@ha"); /* load high 16-bit of the first CF register address */
        asm("li r12, 4"); /* loop 4 times */
        asm("mtctr r12");
        asm("cf_loop:");
        FCCU.CFK.R = FCCU_CFK_KEY; /* set unique key before accessing the flag register */
        /* FCCU.CF_S[0].R = 0xFFFFFFFF; */                                    
        asm ("stw r4, FCCU_CF_S0@l(r3)");/* provide low part 16-bit of the register address */
        asm("addi r3, r3, 4"); /* advance to next register address */
        /* wait for the completion of the operation */
        while(FCCU.CTRL.B.OPS != 0x3)
        /* intentionally put comment here; avoid compiler warning */;
        asm("bdnz cf_loop");

        // clear non-critical faults
        asm ("lis r3, FCCU_NCF_S0@ha");/* load high 16-bit of the first NCF register address */
        asm("li r12, 4"); /* loop 4 times */
        asm("mtctr r12");
        asm("ncf_loop:");
        FCCU.NCFK.R = FCCU_NCFK_KEY; /* set unique key before accessing the flag register */
        /* FCCU.NCF_S[0].R = 0xFFFFFFFF; */
        asm ("stw r4, FCCU_NCF_S0@l(r3)");/* provide low part 16-bit of the register address */
        asm("addi r3, r3, 4");
        /* wait for the completion of the operation */
        while(FCCU.CTRL.B.OPS != 0x3)
        /* intentionally put comment here; avoid compiler warning */;
        asm("bdnz ncf_loop");
        
        /* clear functional/destructive event status registers */
        /* 16-bit registers; flags are cleared with '1' write */
        RGM.FES.R = 0xFFFF;
        RGM.DES.R = 0xFFFF;

        /* re-enter DRUN */
        ME.MCTL.R = 0x30005AF0; /* Enter DRUN Mode & Key */        
        ME.MCTL.R = 0x3000A50F; /* Enter DRUN Mode & Inverted Key */
    }
}

/*------------------------------------------------------------------*/
/* Symbol L2SRAM_LOCATION is defined in the application linker      */
/* command file (.lcf). Its value should reflect SRAM start address.*/ 
/*------------------------------------------------------------------*/
/*lint -esym(752, L2SRAM_LOCATION) */
extern unsigned long L2SRAM_LOCATION;


/*------------------------------------------------------------------*/
/* Symbol L2SRAM_CNT is defined in the application linker command   */
/* file (.lcf). It represents the how many writes with stmw         */
/* (128 bytes each) are needed to cover the whole L2SRAM.           */
/*------------------------------------------------------------------*/
extern unsigned long L2SRAM_CNT;
extern unsigned long L2SRAM_CNT_P1;

MAKE_HLI_COMPATIBLE(SR_WSC_1, 0xc520) /* 50464 */
MAKE_HLI_COMPATIBLE(SR_WSC_2, 0xd928) /* 55592 */
MAKE_HLI_COMPATIBLE(CR_VALUE, 0x8000010A)
MAKE_HLI_COMPATIBLE(SWT_SR, &SWT.SR.R)
MAKE_HLI_COMPATIBLE(SWT_CR, &SWT.CR.R)

/*------------------------------------------------------------------*/
/* FUNCTION     : INIT_Derivative                                   */
/* PURPOSE      : This function initializes the derivative.         */
/*                 Must be executed by core_0.                      */
/* SEQUENCE:                                                        */
/* - initializes main core MMU                                      */
/* - disables system WDT and Core WDT                               */
/* - initializes the internal SRAM for ROM application              */
/*------------------------------------------------------------------*/
__asm void INIT_Derivative(void) 
{
nofralloc

    /* Don't have a stack yet, save the return address in a register */
    mflr     r26;
    bl __initMMU;
    mtlr r26;

    /* Clear the soft lock bit SWT_CR.SLKSWT_CR: */
    /* SR --> 0x0000c520 */
    lis r4, 0
    ori r4, r4, SR_WSC_1@l
    lis r3, SWT_SR@ha
    stw r4, SWT_SR@l(r3)
    /* SR --> 0x0000d928 */
    lis r4, 0
    ori r4, r4, SR_WSC_2@l
    stw r4, SWT_SR@l(r3)

    /* Disable watchdog, SWT.CR.WEN = 0*/
    lis r4, CR_VALUE@h
    ori r4, r4, CR_VALUE@l
    lis r3, SWT_CR@ha
    stw r4, SWT_CR@l(r3)

    mflr r26;
    bl __clearFaultFlags
    mtlr r26;

#ifdef ROM_VERSION
    /* SRAM initialization code */
    lis r11, L2SRAM_LOCATION@h
    ori r11, r11,L2SRAM_LOCATION@l

    /* Loops to cover L2SRAM, stmw allows 128 bytes (32 GPRS x 4 bytes) writes */
    lis r12, L2SRAM_CNT@h
    ori r12, r12, L2SRAM_CNT@l

    start_init:
    mtctr r12
    init_l2sram_loop:
        stmw r0, 0(r11)         /* Write 32 GPRs to SRAM*/
        addi r11, r11, 128      /* Inc the ram ptr; 32 GPRs * 4 bytes = 128B */
        bdnz init_l2sram_loop     /* Loop for L2SRAM_CNT */
#if !defined(LOCKSTEP_MODE) || (LOCKSTEP_MODE == 0)
    /* in decoupled mode initialize the core_1 SRAM also */
    lis r12, L2SRAM_LOCATION_P1@h
    /* check if we already initialized L2SRAM_LOCATION_P1 */
    cmplw r11, r12
    bgt exit_sram_init
    lis r11, L2SRAM_LOCATION_P1@h
    ori r11, r11, L2SRAM_LOCATION_P1@l 
    lis r12, L2SRAM_CNT_P1@h
    ori r12, r12, L2SRAM_CNT_P1@l
    b start_init
    exit_sram_init:
#endif

#endif

    blr
}

/*-------------------------------------------------------------------------*/
/* FUNCTION:WriteMMUTableEntryS                                            */
/* PURPOSE: Creates a new TLB entry with synchronization.It ensures that   */
/*             the TLB context change doesn't affect the core by execution */
/*             before and after tlbwe a CSI. This should be called when    */
/*             creating SRAM or FLASH TLB entries.                         */
/* SEQUENCE: write GPR to MAS, execute CSI, tlbwe, CSI                     */
/*-------------------------------------------------------------------------*/
__asm void WriteMMUTableEntryS( void )
{ 
nofralloc
    /* Write MMU Assist Register 0 (MAS0); SPR 624 */
    mtspr   624, r3
    /* Write MMU Assist Register 1 (MAS1); SPR 625 */
    mtspr   625, r4
    /* Write MMU Assist Register 2 (MAS2); SPR 626 */
    mtspr   626, r5
    /* Write MMU Assist Register 3 (MAS3); SPR 627 */
    mtspr   627, r6
    /* Write the table entry */

    /* All instruction will complete here in current context. */
    msync
    tlbwe
    /* synchronize instruction fetches and data accesses in respect
     with the new created TLB entry. */
    msync
    isync
    blr
}

/*--------------------------------------------------*/
/* FUNCTION:WriteMMUTableEntry                        */
/* PURPOSE: Creates a new TLB entry with             */
/* SEQUENCE: write GPR to MAS, execute tlbwe        */
/*--------------------------------------------------*/
__asm void WriteMMUTableEntry( void )
{ 
nofralloc
    /* Write MMU Assist Register 0 (MAS0); SPR 624 */
    mtspr   624, r3
    /* Write MMU Assist Register 1 (MAS1); SPR 625 */
    mtspr   625, r4
    /* Write MMU Assist Register 2 (MAS2); SPR 626 */
    mtspr   626, r5
    /* Write MMU Assist Register 3 (MAS3); SPR 627 */
    mtspr   627, r6
    /* Write the table entry */
    tlbwe
    blr
}

/*
   Core_0 MMU table:
   -----------------------------------------------------------------
   Name:            TLB entry  Start        Length          Mode    
   -----------------------------------------------------------------
   FLASH            0         0x0000_0000   1MB             LS/DP   
   SHADOW_FLASH     3         0x00F0_0000   1MB             LS/DP   
   (1)SRAM_LS/DP_0  1         0x4000_0000   128KB/64KB      LS/DP   
   (2)SRAM_DP_1     2         0x5000_0000   64KB            DP      
   On-P. Per. 1     4         0x8FF0_0000   512KB           DP      
   (3)Off-P.  A     5         0xC3F8_8000   512KB           LS/DP
   Off-P. Per. B    6         0xFFE0_0000   512KB           LS/DP
   On-P. Per. 0     7         0xFFF0_0000   512KB           LS/DP
   Off-P. Per. C    8         0xFFF9_0000   256KB           LS/DP
   BAM              9         0xFFFF_C000   16KB            LS/DP
   -----------------------------------------------------------------
   (1) allocated for core_0 for both LS and DP modes
   (2) allocated for core_1 for DP mode; core_0 can access 2nd core's SRAM
   (3) Off-platform peripherals mirrored to memory range 0xFFE8_0000–0xFFEF_FFFF
*/

/* Run MMU init:TLB 15, 0x0000_0000, 4GB, TS=1, not guarded, big endian, cache inhibited, all access, VLE/BOOKE */
MAKE_HLI_COMPATIBLE(TLB_Entry_15_MAS0, MAS0_VALUE(15))
MAKE_HLI_COMPATIBLE(TLB_Entry_15_MAS1, MAS1_VALUE(V_VALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_ON, TSIZE_4GB))
MAKE_HLI_COMPATIBLE(TLB_Entry_15_MAS1_INVALID, MAS1_VALUE(V_INVALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_ON, TSIZE_4GB))
#if __option(vle)
MAKE_HLI_COMPATIBLE(TLB_Entry_15_MAS2, MAS2_VALUE(0, VLE_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, NOT_GUARDED, BIG_ENDIAN))
#else
MAKE_HLI_COMPATIBLE(TLB_Entry_15_MAS2, MAS2_VALUE(0, BOOK_E_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, NOT_GUARDED, BIG_ENDIAN))
#endif
MAKE_HLI_COMPATIBLE(TLB_Entry_15_MAS3, MAS3_VALUE(0, READ_WRITE_EXECUTE))

/* FLASH: TLB 0, 0x0000_0000, 1MB, not guarded, cachable, all access, VLE/BOOKE, LS/DP */
MAKE_HLI_COMPATIBLE(TLB_Entry_0_MAS0, MAS0_VALUE(0))
MAKE_HLI_COMPATIBLE(TLB_Entry_0_MAS1, MAS1_VALUE(V_VALID, IPROT_PROTECTED, TID_GLOBAL, TS_OFF, TSIZE_1MB))
#if __option(vle)
MAKE_HLI_COMPATIBLE(TLB_Entry_0_MAS2, MAS2_VALUE(0x00000, VLE_MODE, WRITE_BACK, CACHEABLE, MEM_COHERENCE_NREQ, NOT_GUARDED, BIG_ENDIAN))
#else
MAKE_HLI_COMPATIBLE(TLB_Entry_0_MAS2, MAS2_VALUE(0x00000, BOOK_E_MODE, WRITE_BACK, CACHEABLE, MEM_COHERENCE_NREQ, NOT_GUARDED, BIG_ENDIAN))
#endif
MAKE_HLI_COMPATIBLE(TLB_Entry_0_MAS3, MAS3_VALUE(0x00000, READ_WRITE_EXECUTE))

/* SHADOW_FLASH: TLB 3, 0x00F0_0000, 1MB, not guarded, cachable, all access, BOOKE, LS/DP */
MAKE_HLI_COMPATIBLE(TLB_Entry_3_MAS0, MAS0_VALUE(3))
MAKE_HLI_COMPATIBLE(TLB_Entry_3_MAS1, MAS1_VALUE(V_VALID, IPROT_PROTECTED, TID_GLOBAL, TS_OFF, TSIZE_1MB))
MAKE_HLI_COMPATIBLE(TLB_Entry_3_MAS2, MAS2_VALUE(0xF00000, BOOK_E_MODE, WRITE_BACK, CACHEABLE, MEM_COHERENCE_NREQ, NOT_GUARDED, BIG_ENDIAN))
MAKE_HLI_COMPATIBLE(TLB_Entry_3_MAS3, MAS3_VALUE(0xF00000, READ_WRITE))

#ifdef LOCKSTEP_MODE
/* LS mode */

/* SRAM: TLB 1, 0x4000_0000, 128KB, not protected, not guarded, cache off, big-endian, all access, BOOKE/VLE */
MAKE_HLI_COMPATIBLE(TLB_Entry_1_MAS0, MAS0_VALUE(1))
MAKE_HLI_COMPATIBLE(TLB_Entry_1_MAS1, MAS1_VALUE(V_VALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_OFF, TSIZE_128KB))
#if __option(vle)
MAKE_HLI_COMPATIBLE(TLB_Entry_1_MAS2, MAS2_VALUE(0x40000000, VLE_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, NOT_GUARDED, BIG_ENDIAN))
#else
MAKE_HLI_COMPATIBLE(TLB_Entry_1_MAS2, MAS2_VALUE(0x40000000, BOOK_E_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, NOT_GUARDED, BIG_ENDIAN))
#endif
MAKE_HLI_COMPATIBLE(TLB_Entry_1_MAS3, MAS3_VALUE(0x40000000, READ_WRITE_EXECUTE))

#else
/* DP mode */

/* core_0 RAM space */
/* SRAM: TLB 1, 0x4000_0000, 64KB, not protected, not guarded, cache off, big-endian, all access, BOOKE/VLE */
MAKE_HLI_COMPATIBLE(TLB_Entry_1x_MAS0_core_0, MAS0_VALUE(1)) /* core_0 tlb */
MAKE_HLI_COMPATIBLE(TLB_Entry_1x_MAS0_core_1, MAS0_VALUE(2)) /* core_1 tlb */
MAKE_HLI_COMPATIBLE(TLB_Entry_1x_MAS1, MAS1_VALUE(V_VALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_OFF, TSIZE_64KB))
#if __option(vle)
MAKE_HLI_COMPATIBLE(TLB_Entry_1x_MAS2, MAS2_VALUE(0x40000000, VLE_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, NOT_GUARDED, BIG_ENDIAN))
#else
MAKE_HLI_COMPATIBLE(TLB_Entry_1x_MAS2, MAS2_VALUE(0x40000000, BOOK_E_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, NOT_GUARDED, BIG_ENDIAN))
#endif
MAKE_HLI_COMPATIBLE(TLB_Entry_1x_MAS3, MAS3_VALUE(0x40000000, READ_WRITE_EXECUTE))

/* core_1 RAM space */
/* SRAM: TLB 2, 0x5000_0000, 64KB, not protected, not guarded, cache off, big-endian, all access, BOOKE/VLE */
MAKE_HLI_COMPATIBLE(TLB_Entry_2x_MAS0_core_0, MAS0_VALUE(1)) /* core_1 tlb */
MAKE_HLI_COMPATIBLE(TLB_Entry_2x_MAS0_core_1, MAS0_VALUE(2)) /* core_0 tlb */
MAKE_HLI_COMPATIBLE(TLB_Entry_2x_MAS1, MAS1_VALUE(V_VALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_OFF, TSIZE_64KB))
#if __option(vle)
MAKE_HLI_COMPATIBLE(TLB_Entry_2x_MAS2, MAS2_VALUE(0x50000000, VLE_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, NOT_GUARDED, BIG_ENDIAN))
#else
MAKE_HLI_COMPATIBLE(TLB_Entry_2x_MAS2, MAS2_VALUE(0x50000000, BOOK_E_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, NOT_GUARDED, BIG_ENDIAN))
#endif
MAKE_HLI_COMPATIBLE(TLB_Entry_2x_MAS3, MAS3_VALUE(0x50000000, READ_WRITE_EXECUTE))

/* DP mode only modules */
/* On-P. Per. 1: TLB 4, 0x8FF0_0000, 512KB, not protected, guarded, cache off, big-endian, all access, BOOKE */
MAKE_HLI_COMPATIBLE(TLB_Entry_4_MAS0, MAS0_VALUE(4))
MAKE_HLI_COMPATIBLE(TLB_Entry_4_MAS1, MAS1_VALUE(V_VALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_OFF, TSIZE_512KB))
MAKE_HLI_COMPATIBLE(TLB_Entry_4_MAS2, MAS2_VALUE(0x8FF00000, BOOK_E_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, GUARDED, BIG_ENDIAN))
MAKE_HLI_COMPATIBLE(TLB_Entry_4_MAS3, MAS3_VALUE(0x8FF00000, READ_WRITE))
#endif

/* Off-P. Per. A: TLB 5, 0xC3F8_8000, 512KB, not protected, guarded, cache off, big-endian, all access, BOOKE, LS/DP */
MAKE_HLI_COMPATIBLE(TLB_Entry_5_MAS0, MAS0_VALUE(5))
MAKE_HLI_COMPATIBLE(TLB_Entry_5_MAS1, MAS1_VALUE(V_VALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_OFF, TSIZE_512KB))
MAKE_HLI_COMPATIBLE(TLB_Entry_5_MAS2, MAS2_VALUE(0xC3F88000, BOOK_E_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, GUARDED, BIG_ENDIAN))
MAKE_HLI_COMPATIBLE(TLB_Entry_5_MAS3, MAS3_VALUE(0xC3F88000, READ_WRITE))

/* Off-P. Per. B: TLB 6, 0xFFE0_0000, 512KB, not protected, guarded, cache off, big-endian, all access, BOOKE, LS/DP */
MAKE_HLI_COMPATIBLE(TLB_Entry_6_MAS0, MAS0_VALUE(6))
MAKE_HLI_COMPATIBLE(TLB_Entry_6_MAS1, MAS1_VALUE(V_VALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_OFF, TSIZE_512KB))
MAKE_HLI_COMPATIBLE(TLB_Entry_6_MAS2, MAS2_VALUE(0xFFE00000, BOOK_E_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, GUARDED, BIG_ENDIAN))
MAKE_HLI_COMPATIBLE(TLB_Entry_6_MAS3, MAS3_VALUE(0xFFE00000, READ_WRITE))

/* On-P. Per. 0: TLB 7, 0xFFF0_0000, 512KB, not protected, guarded, cache off, big-endian, all access, BOOKE, LS/DP */
MAKE_HLI_COMPATIBLE(TLB_Entry_7_MAS0, MAS0_VALUE(7))
MAKE_HLI_COMPATIBLE(TLB_Entry_7_MAS1, MAS1_VALUE(V_VALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_OFF, TSIZE_512KB))
MAKE_HLI_COMPATIBLE(TLB_Entry_7_MAS2, MAS2_VALUE(0xFFF00000, BOOK_E_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, GUARDED, BIG_ENDIAN))
MAKE_HLI_COMPATIBLE(TLB_Entry_7_MAS3, MAS3_VALUE(0xFFF00000, READ_WRITE))

/* Off-P. Per. C, DSPI_x, FlexCAN_x: TLB 8, 0xFFF9_0000, 256KB, not protected, guarded, cache off, big-endian, all access, BOOKE, LS/DP */
MAKE_HLI_COMPATIBLE(TLB_Entry_8_MAS0, MAS0_VALUE(8))
MAKE_HLI_COMPATIBLE(TLB_Entry_8_MAS1, MAS1_VALUE(V_VALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_OFF, TSIZE_256KB))
MAKE_HLI_COMPATIBLE(TLB_Entry_8_MAS2, MAS2_VALUE(0xFFF90000, BOOK_E_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, GUARDED, BIG_ENDIAN))
MAKE_HLI_COMPATIBLE(TLB_Entry_8_MAS3, MAS3_VALUE(0xFFF90000, READ_WRITE))

/* Off-P. Per. C, eDMA, FlexRay: TLB 9, 0xFFFD_0000, 128KB, not protected, guarded, cache off, big-endian, all access, BOOKE, LS/DP */
MAKE_HLI_COMPATIBLE(TLB_Entry_9_MAS0, MAS0_VALUE(9))
MAKE_HLI_COMPATIBLE(TLB_Entry_9_MAS1, MAS1_VALUE(V_VALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_OFF, TSIZE_128KB))
MAKE_HLI_COMPATIBLE(TLB_Entry_9_MAS2, MAS2_VALUE(0xFFFD0000, BOOK_E_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, GUARDED, BIG_ENDIAN))
MAKE_HLI_COMPATIBLE(TLB_Entry_9_MAS3, MAS3_VALUE(0xFFFD0000, READ_WRITE))

/* BAM: TLB 10, 0xFFFF_C000, 16KB, not protected, guarded, cache off, big-endian, all access, VLE, LS/DP */
MAKE_HLI_COMPATIBLE(TLB_Entry_10_MAS0, MAS0_VALUE(10))
MAKE_HLI_COMPATIBLE(TLB_Entry_10_MAS1, MAS1_VALUE(V_VALID, IPROT_NOTPROTECTED, TID_GLOBAL, TS_OFF, TSIZE_16KB))
MAKE_HLI_COMPATIBLE(TLB_Entry_10_MAS2, MAS2_VALUE(0xFFFFC000, VLE_MODE, WRITE_BACK, CACHE_INHIBIT, MEM_COHERENCE_NREQ, GUARDED, BIG_ENDIAN))
MAKE_HLI_COMPATIBLE(TLB_Entry_10_MAS3, MAS3_VALUE(0xFFFFC000, READ_EXECUTE))
/* don't allow second core to execute from BAM */
MAKE_HLI_COMPATIBLE(TLB_Entry_10_MAS3_core_1, MAS3_VALUE(0xFFFFC000, READ))

__asm void __initMMU(void)
{
nofralloc
    /* Use a translation address space page in order to run the MMU initialization
      regardless of the current executing env. and MMU setup. */
    lis r3, TLB_Entry_15_MAS0@h
    ori r3, r3, TLB_Entry_15_MAS0@l
    mtspr 624, r3
    lis r4, TLB_Entry_15_MAS1@h
    ori r4, r4, TLB_Entry_15_MAS1@l
    mtspr 625, r4
    xor r5, r5, r5
    mr r6, r5
    ori r5, r5, TLB_Entry_15_MAS2@l
    ori r6, r6, TLB_Entry_15_MAS3@l
    mtspr 626, r5
    mtspr 627, r6
    tlbwe

    /* force this TLB entry to be used for translation */
    mfmsr r10
    /* save state */
    mr r3, r10
    /* set IS=1, DS=1 */
    ori r3, r3, 0x20

    msync
    /* mtmsr does execution synchronization.*/
    mtmsr r3
    /* Required after changing MSR.IS, and MSR.DS so the prefetched instructions
      will be discarded and all subsequent instructions will use the TLB 15 context.*/
    isync
    msync

     /* FLASH TLB0 0x0000_0000 1MB LS/DP */
     lis r3, TLB_Entry_0_MAS0@h
     ori r3, r3, TLB_Entry_0_MAS0@l
     lis r4, TLB_Entry_0_MAS1@h
     ori r4, r4, TLB_Entry_0_MAS1@l
     lis r5, TLB_Entry_0_MAS2@h
     ori r5, r5, TLB_Entry_0_MAS2@l
     lis r6, TLB_Entry_0_MAS3@h
     ori r6, r6, TLB_Entry_0_MAS3@l
     /* Synchronize in case running from FLASH */
     mflr r28
     bl WriteMMUTableEntryS
     mtlr r28

     /* SHADOW_FLASH,TLB3,0x00F0_0000,1MB,LS/DP*/
     lis r3, TLB_Entry_3_MAS0@h
     ori r3, r3, TLB_Entry_3_MAS0@l
     lis r4, TLB_Entry_3_MAS1@h
     ori r4, r4, TLB_Entry_3_MAS1@l
     lis r5, TLB_Entry_3_MAS2@h
     ori r5, r5, TLB_Entry_3_MAS2@l
     lis r6, TLB_Entry_3_MAS3@h
     ori r6, r6, TLB_Entry_3_MAS3@l
     mflr r28
     bl WriteMMUTableEntry
     mtlr r28

#ifdef LOCKSTEP_MODE
     /* RAM_LS,TLB 1,0x4000_0000,128KB,LS,core_0 */
     lis r3, TLB_Entry_1_MAS0@h
     ori r3, r3, TLB_Entry_1_MAS0@l
     lis r4, TLB_Entry_1_MAS1@h
     ori r4, r4, TLB_Entry_1_MAS1@l
     lis r5, TLB_Entry_1_MAS2@h
     ori r5, r5, TLB_Entry_1_MAS2@l
     lis r6, TLB_Entry_1_MAS3@h
     ori r6, r6, TLB_Entry_1_MAS3@l
     mflr r28
     bl WriteMMUTableEntryS
     mtlr r28

#else
     /* DP mode */
     /* SRAM_DP_0,TLB1,0x4000_0000,64KB,DP,core_0 */
     /* SRAM_DP_0,TLB2,0x4000_0000,64KB,DP,core_1 */

     /* check core id */
     mfpir r25
     cmpi r25, 0
     bne ram_0_core_1
     lis r3, TLB_Entry_1x_MAS0_core_0@h
     ori r3, r3, TLB_Entry_1x_MAS0_core_0@l
     b ram_0
     ram_0_core_1:
     lis r3, TLB_Entry_1x_MAS0_core_1@h
     ori r3, r3, TLB_Entry_1x_MAS0_core_1@l
     ram_0:
     lis r4, TLB_Entry_1x_MAS1@h
     ori r4, r4, TLB_Entry_1x_MAS1@l
     lis r5, TLB_Entry_1x_MAS2@h
     ori r5, r5, TLB_Entry_1x_MAS2@l
     lis r6, TLB_Entry_1x_MAS3@h
     ori r6, r6, TLB_Entry_1x_MAS3@l
     mflr r28
     bl WriteMMUTableEntryS
     mtlr r28

     /* SRAM_DP_1,TLB2,0x5000_0000,64KB,DP,core_0 */
     /* SRAM_DP_1,TLB1,0x5000_0000,64KB,DP,core_1 */
     mfpir r25
     cmpi r25, 0
     bne ram_1_core_0
     lis r3, TLB_Entry_2x_MAS0_core_1@h
     ori r3, r3, TLB_Entry_2x_MAS0_core_1@l
     b ram_1
     ram_1_core_0:
     lis r3, TLB_Entry_2x_MAS0_core_0@h
     ori r3, r3, TLB_Entry_2x_MAS0_core_0@l
     ram_1:
     lis r4, TLB_Entry_2x_MAS1@h
     ori r4, r4, TLB_Entry_2x_MAS1@l
     lis r5, TLB_Entry_2x_MAS2@h
     ori r5, r5, TLB_Entry_2x_MAS2@l
     lis r6, TLB_Entry_2x_MAS3@h
     ori r6, r6, TLB_Entry_2x_MAS3@l
     mflr r28
     bl WriteMMUTableEntryS
     mtlr r28

     /*On-P. Per. 1, TLB4,0x8FF0_0000,512KB,DP*/
     lis r3, TLB_Entry_4_MAS0@h
     ori r3, r3, TLB_Entry_4_MAS0@l
     lis r4, TLB_Entry_4_MAS1@h
     ori r4, r4, TLB_Entry_4_MAS1@l
     lis r5, TLB_Entry_4_MAS2@h
     ori r5, r5, TLB_Entry_4_MAS2@l
     lis r6, TLB_Entry_4_MAS3@h
     ori r6, r6, TLB_Entry_4_MAS3@l
     mflr r28
     bl WriteMMUTableEntry
     mtlr r28
#endif

     /* Off-P. Per. A, TLB5, 0xC3F8_8000,512KB,LS/DP */
     lis r3, TLB_Entry_5_MAS0@h
     ori r3, r3, TLB_Entry_5_MAS0@l
     lis r4, TLB_Entry_5_MAS1@h
     ori r4, r4, TLB_Entry_5_MAS1@l
     lis r5, TLB_Entry_5_MAS2@h
     ori r5, r5, TLB_Entry_5_MAS2@l
     lis r6, TLB_Entry_5_MAS3@h
     ori r6, r6, TLB_Entry_5_MAS3@l
     mflr r28
     bl WriteMMUTableEntry
     mtlr r28

     /* Off-P. Per. B,TLB6,0xFFE0_0000,512KB,LS/DP */
     lis r3, TLB_Entry_6_MAS0@h
     ori r3, r3, TLB_Entry_6_MAS0@l
     lis r4, TLB_Entry_6_MAS1@h
     ori r4, r4, TLB_Entry_6_MAS1@l
     lis r5, TLB_Entry_6_MAS2@h
     ori r5, r5, TLB_Entry_6_MAS2@l
     lis r6, TLB_Entry_6_MAS3@h
     ori r6, r6, TLB_Entry_6_MAS3@l
     mflr r28
     bl WriteMMUTableEntry
     mtlr r28

     /* On-P. Per. 0, TLB7, 0xFFF0_0000, 512KB,LS/DP */
     lis r3, TLB_Entry_7_MAS0@h
     ori r3, r3, TLB_Entry_7_MAS0@l
     lis r4, TLB_Entry_7_MAS1@h
     ori r4, r4, TLB_Entry_7_MAS1@l
     lis r5, TLB_Entry_7_MAS2@h
     ori r5, r5, TLB_Entry_7_MAS2@l
     lis r6, TLB_Entry_7_MAS3@h
     ori r6, r6, TLB_Entry_7_MAS3@l
     mflr r28
     bl WriteMMUTableEntry
     mtlr r28

     /* Off-P. Per. C, DSPI_x, FlexCAN_x: TLB 8, 0xFFF9_0000, 256KB, LS/DP */
     lis r3, TLB_Entry_8_MAS0@h
     ori r3, r3, TLB_Entry_8_MAS0@l
     lis r4, TLB_Entry_8_MAS1@h
     ori r4, r4, TLB_Entry_8_MAS1@l
     lis r5, TLB_Entry_8_MAS2@h
     ori r5, r5, TLB_Entry_8_MAS2@l
     lis r6, TLB_Entry_8_MAS3@h
     ori r6, r6, TLB_Entry_8_MAS3@l
     mflr r28
     bl WriteMMUTableEntry
     mtlr r28

     /* Off-P. Per. C, eDMA, FlexRay: TLB 9, 0xFFFD_0000, 128KB, LS/DP */
     lis r3, TLB_Entry_9_MAS0@h
     ori r3, r3, TLB_Entry_9_MAS0@l
     lis r4, TLB_Entry_9_MAS1@h
     ori r4, r4, TLB_Entry_9_MAS1@l
     lis r5, TLB_Entry_9_MAS2@h
     ori r5, r5, TLB_Entry_9_MAS2@l
     lis r6, TLB_Entry_9_MAS3@h
     ori r6, r6, TLB_Entry_9_MAS3@l
     mflr r28
     bl WriteMMUTableEntry
     mtlr r28

     /* BAM,TLB10,0xFFFC_0000,16KB,LS/DP, R-X, core_0 */
     /* BAM,TLB10,0xFFFC_0000,16KB,LS/DP, R--, core_1 */
     lis r3, TLB_Entry_10_MAS0@h
     ori r3, r3, TLB_Entry_10_MAS0@l
     lis r3, TLB_Entry_10_MAS0@h
     ori r3, r3, TLB_Entry_10_MAS0@l
     lis r4, TLB_Entry_10_MAS1@h
     ori r4, r4, TLB_Entry_10_MAS1@l
     lis r5, TLB_Entry_10_MAS2@h
     ori r5, r5, TLB_Entry_10_MAS2@l
     mfpir r25
     cmpi r25, 0
     bne bam_core_1
     lis r6, TLB_Entry_10_MAS3@h
     ori r6, r6, TLB_Entry_10_MAS3@l
     b bam_tlb
     bam_core_1:
     lis r6, TLB_Entry_10_MAS3_core_1@h
     ori r6, r6, TLB_Entry_10_MAS3_core_1@l
     bam_tlb:
     mflr r28
     bl WriteMMUTableEntry
     mtlr r28

     /* restore msr */
     mtmsr r10
     /* dicard instruction & data prefetch */
     msync

     /* invalidated initialization TLB entry 15 */
     lis r3, TLB_Entry_15_MAS0@h
     ori r3, r3, TLB_Entry_15_MAS0@l
     lis r4, TLB_Entry_15_MAS1_INVALID@h
     ori r4, r4, TLB_Entry_15_MAS1_INVALID@l
     mtspr 624, r3
    mtspr 625, r4
    tlbwe
     /* make sure isntructions and data are fetched from the new context. */
     isync
     msync

     blr
}

/*----------------------------------------------------------------------*/
/* Initialize the needed MMU Table entries for core_1                   */
/* Note: The function is places inside the .init section so both        */
/* cores can execute the same MMU init code and have the same memory    */ 
/* space view.                                                          */
/*----------------------------------------------------------------------*/
__declspec(section ".init") __asm void __initMMU_p1(void)
{
fralloc
    mflr r27
    bl __initMMU
    mtlr r27
    blr
}

/*--------------------------------------------------------------------------*/
/* FUNCTION     : INIT_ExternalBusAndMemory                                 */
/* PURPOSE      : This function configures EBI for the external memory      */
/* SEQUENCE: none                                                           */
/*--------------------------------------------------------------------------*/
__asm void INIT_ExternalBusAndMemory(void) 
{
    nofralloc
    blr
}

#ifdef __cplusplus
}
#endif
