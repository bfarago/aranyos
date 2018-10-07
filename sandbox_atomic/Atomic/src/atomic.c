/*
 * atomic.c
 * Atomic operations for PPC e200z4, implementation
 *  Created on: Oct 6, 2018
 *      Author: Barna Faragó
 */
#include "atomic.h"
#include "MPC5643L.h"

__asm uint32_t Atomic_Inc_u32(vuint32_t* ptr)
{
	nofralloc
	// r3 : ptr
loop:
	lwarx   r5,0,r3          // Load and reserve
	addic	r5,r5,1			 // Increment r5 reg.
	stwcx.  r5,0,r3          // Store new value if still reserved
	bne-    loop             // Loop if lost reservation
	blr
}
__asm uint32_t Atomic_Dec_u32(vuint32_t* ptr)
{
	nofralloc
	// r3 : ptr
loop:
	lwarx   r5,0,r3          // Load and reserve
	addic	r5,r5,-1		 // Increment r5 reg.
	stwcx.  r5,0,r3          // Store new value if still reserved
	bne-    loop             // Loop if lost reservation
	blr
}
__asm uint32_t Atomic_Inc_u8(vuint8_t* ptr)
{
	nofralloc
	// r3 : ptr
loop:
	lbarx   r5,0,r3          // Load and reserve
	addic	r5,r5,1			 // Increment r5 reg.
	stbcx.  r5,0,r3          // Store new value if still reserved
	bne-    loop             // Loop if lost reservation
	blr
}
__asm uint32_t Atomic_Dec_u8(vuint8_t* ptr)
{
	nofralloc
	// r3 : ptr
loop:
	lbarx   r5,0,r3          // Load and reserve
	addic	r5,r5,-1			 // Increment r5 reg.
	stbcx.  r5,0,r3          // Store new value if still reserved
	bne-    loop             // Loop if lost reservation
	blr
}

__asm uint32_t Atomic_Store_u32(uint32_t prev, uint32_t next, vuint32_t*addr )
{
	// r3 : prev, r4: next, r5:addr. return: r3
	//nofralloc 	//manually blr from func., no stack needed.
retry:
	lwarx  r6, 0, r5 // current = *addr;
	cmpw   r6, r3    // if( current != prev )
	bne    fail      // goto fail;
	stwcx. r4, 0, r5 // if( reservation == addr ) *addr = next;
	bne-   retry     // else goto retry;
	li      r3, 1    // Return true.
	blr              // We're outta here.
fail:
	stwcx. r6, 0, r5 // Clear reservation.
	li     r3, 0     // Return false.
	blr              // We're outta here.
}

__asm uint32_t Atomic_Store_u8(uint8_t prev, uint8_t next, vuint8_t*addr )
{
	// r3 : prev, r4: next, r5:addr. return: r3
	//nofralloc 	//manually blr from func., no stack needed.
retry:
	lbarx  r6, 0, r5 // current = *addr;
	cmpw   r6, r3    // if( current != prev )
	bne    fail      // goto fail;
	stbcx. r4, 0, r5 // if( reservation == addr ) *addr = next;
	bne-   retry     // else goto retry;
	li      r3, 1    // Return true.
	blr              // We're outta here.
fail:
	stbcx. r6, 0, r5 // Clear reservation.
	li     r3, 0     // Return false.
	blr              // We're outta here.
}
/* Get Processor ID */
__asm uint8_t getPir(void){
	mfspr r3, 286
}

/* Aquire SEMA4 */
uint32_t aquireLock(int gate){
	uint8_t   locked_value, current_value;
	locked_value=(getPir() & 1) + 1;
	 /* read the current value of the gate and wait until the state == UNLOCK */
	do {
		current_value = SEMA4.GATE[gate].B.GTFSM;
	} while (current_value != 0);
	/* the current value of the gate == UNLOCK. attempt to lock the gate for this
    processor. spin-wait in this loop until gate ownership is obtained */
	do {
		SEMA4.GATE[gate].B.GTFSM = locked_value; /* write gate with processor_number + 1 */
		current_value =  SEMA4.GATE[gate].B.GTFSM; /* read gate to verify ownership was obtained */  
	} while (current_value != locked_value);
	return 1;
}

/* Release SEMA4 */
void releaseLock(int gate){
	/* Only the MASTER can write the register, which locked.
	Writing zero to gate register, release the lock.
	*/
	SEMA4.GATE[gate].B.GTFSM = 0;
}
