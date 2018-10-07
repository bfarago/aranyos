/*
 * shared.c
 * Example for shared Atomic variable handling
 *  Created on: Oct 7, 2018
 *      Author: Barna
 */
#include "shared.h"
#include "atomic.h"

vuint32_t shared_counters[4];
uint32_t shared_retry;


//this code can be run from both cpu core, in same time :).
void shared_task(void)
{	
	uint32_t stored=0;
	uint32_t retry=0;
	unsigned int coreID;
	coreID = getPir();
	
	aquireLock(0); //gate 0 is used for this task
	do{
		vuint32_t data = shared_counters[2];
		stored=Atomic_Store_u32(data, data+1, &(shared_counters[2]) ); //possible changed by other core only.
		if (!stored) retry++; //shound be zero, even id multicore. May nonzero if local IRQ hit after reservation.
	}while (!stored); //protects both cores
	releaseLock(0);
	
	if (retry) shared_retry+=retry;
	Atomic_Inc_u32(&shared_counters[coreID]); //protects only in case of core's isr changed it.
	shared_counters[3]=shared_counters[0]+shared_counters[1];
}
