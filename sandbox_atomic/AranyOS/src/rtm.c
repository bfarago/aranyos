/*
 * rtm.c
 * Runtime measurement, implementation
 *  Created on: Oct 7, 2018
 *      Author: Barna Farago
 */

#include "rtm.h"
#include "MPC5643L.h"

#define SPR_TBL 284
#define SPR_HID0 1008
#define HID0_TBEN 0x4000

uint32_t getTimeStamp32();

__asm uint32_t getTimeStamp32()
{
	mfspr r3, SPR_TBL
}
__asm void Rtm_Init()
{
	mfspr r3, SPR_HID0
	e_or2i r3, HID0_TBEN
	mtspr SPR_HID0,r3
}
void Rtm_Clear(rtm_var_s* p)
{
	p->delta=0;
	p->minnet=0xFFFFFFFFu;
	p->maxnet=0;
	p->avgnet=0;
}
void Rtm_Start(rtm_var_s* p)
{
	p->ts0=	getTimeStamp32();
	p->ts1=p->ts0;
	p->delta=0;
}
void Rtm_Pause(rtm_var_s* p)
{
	uint32_t now= getTimeStamp32();
	uint32_t delta;
	if (now > p->ts1){
		delta= now - p->ts1;
	}else{
		delta= p->ts1-now;
	}
	
	p->delta+= delta;
	p->ts1=now;
}
void Rtm_Resume(rtm_var_s* p)
{
	p->ts1=	getTimeStamp32();
}
void Rtm_Stop(rtm_var_s* p)
{
	Rtm_Pause(p);
	p->gross= p->ts1 - p->ts0; //todo: store 64 bit ?
	p->net= p->delta;
	if(p->maxnet< p->net) p->maxnet= p->net;
	if(p->minnet> p->net) p->minnet= p->net;
	p->avgnet=(p->maxnet+p->minnet)>>1;
}
void Rtm_Decay(rtm_var_s* p)
{
	if (p->maxnet>p->avgnet) p->maxnet--;
	if (p->minnet<p->avgnet) p->minnet++;
}
