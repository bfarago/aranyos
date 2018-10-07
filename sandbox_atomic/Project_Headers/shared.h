/*
 * shared.h
 *
 *  Created on: Oct 7, 2018
 *      Author: Barna
 */

#ifndef SHARED_H_
#define SHARED_H_
#include "typedefs.h"
extern vuint32_t shared_counters[4];

#pragma push

#pragma section code_type ".text_vle" data_mode=far_abs code_mode=far_abs
extern void shared_task(void);

#pragma pop

#endif /* SHARED_H_ */
