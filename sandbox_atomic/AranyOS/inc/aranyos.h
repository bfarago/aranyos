/*
 * aranyos.h
 *
 *  Created on: Oct 6, 2018
 *      Author: Barna
 */

#ifndef ARANYOS_H_
#define ARANYOS_H_
#include "typedefs.h"
#include "aranyos_cfg.h"

// Function pointer type for Entry points
typedef void (*AranyOs_Entry_p)(void);

//Task variables type
typedef struct {
	uint8_t activation;
	uint8_t enabled;
	AranyOs_Entry_p entry_p;
} Aranyos_TaskVar_s;

//Task config type
typedef struct {
	uint32_t maskCounter;
	uint32_t valueCounter;
	uint8_t maxActivation;
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
// // const Aranyos_TaskCfg_s TaskCfg_##name; \

#define ISR(name) \
	void ISR_##name(void);\
	void ISR_##name(void)

void Os_Init(void);

void Os_EnableIrq(void);
void Os_DisableIrq(void);

uint32_t Os_CriticalStart(void);
void Os_CriticalEnd(uint32_t state);

uint32_t Os_TaskInitEntry(uint32_t id, AranyOs_Entry_p entry_p, Aranyos_TaskVar_s* var_p, const Aranyos_TaskCfg_s* cfg_p );
void Os_TaskActivation(uint32_t id);
void Os_TaskSetEnable(uint32_t id, uint8_t enabled);
void Os_Scheduler(void);
void Os_Periodic(void);


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
typedef enum{
	OS_DET_ERROR_ID_WrongArgument,
	OS_DET_ERROR_ID_NotInitialized
};

#endif /* ARANYOS_H_ */
