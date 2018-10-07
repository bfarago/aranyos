/*
 * aranyos.c
 *
 *  Created on: Oct 7, 2018
 *      Author: Barna
 */
#include "aranyos.h"
#include "MPC5643L.h"
#include "atomic.h"
#include "det.h"

/* Globals */
uint32_t g_OsCounter;
uint32_t g_OsTasksMax=0;
Aranyos_TaskBook_s g_OsTaskBook[ARANYOS_MAX_TASKS];

/* Os_EnableIrq
 * Lower ITC prio, enable irq.
 */
void Os_EnableIrq(void)
{
	INTC.CPR.B.PRI = 0; /* Single Core: Lower INTC's current priority */
	asm(" wrteei 1"); /* Enable external interrupts */
}
/* Os_EnableIrq
 * Lower ITC prio, enable irq.
 */
void Os_DisableIrq(void)
{
	INTC.CPR.B.PRI = 16; /* Single Core: Rise INTC's current priority */
	asm(" wrteei 0"); /* Enable external interrupts */
}
uint32_t Os_CriticalStart(void)
{
	uint32_t r;
	r=INTC.CPR.B.PRI;
	INTC.CPR.B.PRI=16;
	return r;
}
void Os_CriticalEnd(uint32_t state)
{
	INTC.CPR.B.PRI=state;
}
void Os_Init(void)
{
	int i;
	g_OsTasksMax=0;
	for(i=0; i< ARANYOS_MAX_TASKS; i++)
	{
		Aranyos_TaskBook_s* p= &g_OsTaskBook[i];
		p->cfg_p=0;
		p->var_p=0;
	}
}
//not reentrant, must call from main thread
uint32_t Os_TaskInitEntry(uint32_t id, AranyOs_Entry_p entry_p, Aranyos_TaskVar_s* var_p, const Aranyos_TaskCfg_s* cfg_p )
{
	uint32_t st;
	Aranyos_TaskBook_s* p= &g_OsTaskBook[id];
	if (id >= g_OsTasksMax) g_OsTasksMax=id+1;
	st=Os_CriticalStart();
	{
		p->var_p= var_p;
		p->var_p->entry_p= entry_p;
		p->var_p->activation=0;
		p->var_p->enabled= 1;
		p->cfg_p= cfg_p;
	}
	Os_CriticalEnd(st);
	return g_OsTasksMax;
}
void Os_Periodic(void)
{
	Atomic_Inc_u32(&g_OsCounter);
}
void Os_TaskSetEnable(uint32_t id, uint8_t enabled)
{
	Aranyos_TaskBook_s* p= &g_OsTaskBook[id];
	uint32_t stored=0;
	do{
		stored=Atomic_Store_u8(p->var_p->enabled, enabled, &(p->var_p->enabled));
	}while (!stored);
}

void Os_TaskActivation(uint32_t id)
{
	if (id<g_OsTasksMax){
		Aranyos_TaskBook_s* p= &g_OsTaskBook[id];
		Atomic_Inc_u8(&(p->var_p->activation));
		// Os_TaskSetEnable(id, 1);
	}else{
		Os_Det_ReportError(OS_DET_API_ID_TaskActivation ,OS_DET_ERROR_ID_WrongArgument);
	}
}


static void Os_CheckOneById(uint32_t id)
{
	Aranyos_TaskBook_s* p= &g_OsTaskBook[id];
	Aranyos_TaskVar_s* var= p->var_p;
	const Aranyos_TaskCfg_s* cfg= p->cfg_p;
	if (!var) return; // this task was not initialized
	if (!var->enabled) return; //this task was not enabled

	if ((g_OsCounter & cfg->maskCounter)==cfg->valueCounter)
	{
		//Os_TaskActivation(id); // this service can be called outside, better to use Inc here:
		Atomic_Inc_u8(&(var->activation)); //var->activation++;
	}
	if (var->activation)
	{
		Atomic_Dec_u8(&(var->activation)); //var->activation--;
		var->entry_p();
	}
}
void Os_Scheduler(void)
{
	static uint32_t prevCounter=0;
	uint32_t i;
	if (prevCounter!=g_OsCounter)
	{	//trigger when changed
		prevCounter=g_OsCounter;
		for(i=0; i< g_OsTasksMax; i++)
		{
			Os_CheckOneById(i);
		}
	}
}
