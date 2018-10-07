/*
 * rtm.h
 * Runtime measurement
 *  Created on: Oct 7, 2018
 *      Author: Barna Farago
 */

#ifndef RTM_H_
#define RTM_H_

#include "typedefs.h"

typedef struct{
	uint32_t ts0;
	uint32_t ts1;
	uint32_t delta;
	uint32_t gross;
	uint32_t net;
	uint32_t maxnet;
	uint32_t minnet;
	uint32_t avgnet;
} rtm_var_s;

//HW or CPU Timebase init
void Rtm_Init();
//Clear data
void Rtm_Clear(rtm_var_s* p);
//Start Runtime measurement
void Rtm_Start(rtm_var_s* p);
void Rtm_Pause(rtm_var_s* p);
void Rtm_Resume(rtm_var_s* p);
void Rtm_Stop(rtm_var_s* p);
void Rtm_Decay(rtm_var_s* p);

#endif /* RTM_H_ */
