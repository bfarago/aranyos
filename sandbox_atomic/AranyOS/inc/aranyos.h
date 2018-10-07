/*
 * aranyos.h
 * AranyOS API header file
 *  Created on: Oct 6, 2018
 *      Author: Barna
 */

#ifndef ARANYOS_H_
#define ARANYOS_H_
#include "typedefs.h"
#include "aranyos_cfg.h"
#include "rtm.h"

// Function pointer type for Entry points
typedef void (*AranyOs_Entry_p)(void);

//Task variables type
typedef struct {
	AranyOs_Entry_p entry_p;
	rtm_var_s rtm;
	uint8_t state;
	uint8_t activation;
	uint8_t enabled;
	uint8_t measure;	
} Aranyos_TaskVar_s;

//Task config type, can be const.
typedef struct {
	uint32_t maskCounter;
	uint32_t valueCounter;
	uint8_t maxActivation;
	uint8_t prio;
} Aranyos_TaskCfg_s;

//Task book keeping, internal
typedef struct {
	Aranyos_TaskVar_s * var_p;
	const Aranyos_TaskCfg_s * cfg_p;
} Aranyos_TaskBook_s;

#define TASK(name) \
	Aranyos_TaskVar_s TaskVar_##name; \
	void Entry_##name(void); \
	void Entry_##name(void)

#define ISR(name) \
	void ISR_##name(void);\
	void ISR_##name(void)

//Init Os. Call this first.
void Os_Init(void);

//OS level Enable IRQ
void Os_EnableIrq(void);
//OS level Disable IRQ
void Os_DisableIrq(void);

//OS level Critical section begin
uint32_t Os_CriticalStart(void);
//OS level Critical section end
void Os_CriticalEnd(uint32_t state);

//Add one task to the book-keeping of the OS.
uint32_t Os_TaskInitEntry(uint32_t id, AranyOs_Entry_p entry_p, Aranyos_TaskVar_s* var_p, const Aranyos_TaskCfg_s* cfg_p );
//Activate Task. Task will run next possible schedule point.
void Os_TaskActivation(uint32_t id);
//Set or clear Enable flag of a Task.
void Os_TaskSetEnable(uint32_t id, uint8_t enabled);
//Os scheduler and also fire a task, if activated. (not pre-empt.)
void Os_Scheduler(void);
//Os counter handler, must be called from a periodic timer interrupt.
void Os_Periodic(void);

//DET API service ids
typedef enum{
	OS_DET_API_ID_Init,
	OS_DET_API_ID_EnableIrq,
	OS_DET_API_ID_DisableIrq,
	OS_DET_API_ID_CriticalStart,
	OS_DET_API_ID_CriticalEnd,
	OS_DET_API_ID_TaskInitEntry,	
	OS_DET_API_ID_TaskActivation,
	OS_DET_API_ID_TaskSetEnable,
	OS_DET_API_ID_Scheduler,
	OS_DET_API_ID_Periodic		
};
//DET Error codes
typedef enum{
	OS_DET_ERROR_ID_WrongArgument,
	OS_DET_ERROR_ID_NotInitialized
};

#endif /* ARANYOS_H_ */
