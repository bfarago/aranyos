/*
 * atomic.h
 * Atomic operations for PPC e200z4
 *  Created on: Oct 6, 2018
 *      Author: Barna Faragó
 */

#ifndef ATOMIC_H_
#define ATOMIC_H_
#include "typedefs.h"

/** Atomic_Inc_u8
 * Atomic Increment data, no critical section is needed.
 * ptr: pointer to uint8_t value
 */
uint32_t Atomic_Inc_u8(vuint8_t* ptr);
uint32_t Atomic_Dec_u8(vuint8_t* ptr);

/** Atomic_Inc_u32
 * Atomic Increment data, no critical section is needed.
 * ptr: pointer to uint32_t value
 */
uint32_t Atomic_Inc_u32(vuint32_t* ptr);
uint32_t Atomic_Dec_u32(vuint32_t* ptr);

/** Atomic_Store_u32
 * Atomic Store data, no critical section is needed.
 * prev: previous value
 * next: storable value
 * addr: pointer to the data
 * return 1:success, 0: interrupted/failure 
 */
uint32_t Atomic_Store_u32(uint32_t prev, uint32_t next, vuint32_t *addr );
uint32_t Atomic_Store_u8(uint8_t prev, uint8_t next, vuint8_t*addr );

/* Get Processor ID */
uint8_t getPir(void);

/* Aquire SEMA4 */
uint32_t aquireLock(int gate);

/* Release SEMA4 */
void releaseLock(int gate);

#endif /* ATOMIC_H_ */
