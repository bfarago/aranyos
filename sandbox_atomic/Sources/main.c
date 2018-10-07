/*
 SANDBOX TEST FOR MULTICORE ATOMIC FUNCTIONS
*/
#include "MPC5643L.h"
#include "aranyos.h"
#include "atomic.h"
#include "IntcInterrupts.h"
#include "shared.h"
#include "rtm.h"

//#define PIT_ENABLED // not yet possible
#define STM_ENABLED //used for os clock counter

#if ROM_VERSION == 1
/* Prototype for second core startup */
extern void __start_p1();
#endif

/* Prototype for local functions */
void init(void);

/* Globals for core0*/
volatile uint32_t g_counters[4];
rtm_var_s g_rtm_quick;
TASK(TaskQuick)
{
	Rtm_Pause(&TaskVar_TaskQuick.rtm);
	Rtm_Start(&g_rtm_quick);
	Rtm_Resume(&(TaskVar_TaskQuick.rtm));
	
	Atomic_Inc_u32(&g_counters[0]); //common
	Atomic_Inc_u32(&g_counters[2]); //private
	shared_task();
	
	Rtm_Pause(&(TaskVar_TaskQuick.rtm));
	Rtm_Stop(&g_rtm_quick);
	Rtm_Resume(&(TaskVar_TaskQuick.rtm));
}

TASK(TaskSlow)
{
	Atomic_Inc_u32(&g_counters[0]);
	Atomic_Inc_u32(&g_counters[3]); //private
	Rtm_Decay(&g_rtm_quick);
}

#ifdef STM_ENABLED
ISR(Stm0)
{
	Os_Periodic();
	//test
	Atomic_Inc_u32(&g_counters[0]);
	Atomic_Inc_u32(&g_counters[1]); //private
	//end of ISR
	STM.CNT.R=0; //Reset System Timers Counter
	STM.CHANNEL[0].CIR.R=1; // Interrupt acknowladge for STM
}
#endif

#ifdef PIT_ENABLED
ISR(Pit)
{
	Os_Periodic();
    /* toggle LED */
    // SIU.GPDO[LED3_pin].R ^= 1;
	g_OsCounter++;
	// PIT.TFLG0.R = 0x00000001;
	PIT.CH[0].TFLG.B.TIF=1;
}
#endif

// const Os task config, stored on flash
const Aranyos_TaskCfg_s TaskCfg_TaskQuick= {0x2f,0,3,3};
const Aranyos_TaskCfg_s TaskCfg_TaskSlow=  {0x2ff,0,3,2};

void init(void)
{
	#if ROM_VERSION == 1
	/* Start the second core, VLE mode*/
	SSCM.DPMBOOT.R = (unsigned long)__start_p1 + 0x00000002;
	SSCM.DPMKEY.R = 0x00005AF0;
	SSCM.DPMKEY.R = 0x0000A50F;
	#endif

	Os_DisableIrq();
	// configure os, add two tasks
	Os_Init();
	Os_TaskInitEntry(0, Entry_TaskQuick, &TaskVar_TaskQuick, &TaskCfg_TaskQuick);
	Os_TaskInitEntry(1, Entry_TaskSlow, &TaskVar_TaskSlow, &TaskCfg_TaskSlow);

	// configure some timer for os
#ifdef STM_ENABLED
	INTC_InstallINTCInterruptHandler(ISR_Stm0, 30, 2); //PIT CH 0 INTC
	STM.CR.B.FRZ=1;
	STM.CR.B.CPS=120;
	STM.CHANNEL[0].CCR.B.CEN =1;
	STM.CHANNEL[0].CIR.B.CIF=1;
	STM.CHANNEL[0].CMP.R=200;
	STM.CR.B.TEN=1;
#endif	

#ifdef PIT_ENABLED
	// this one needs some more MC/clk config ? we got load-store exception now. 
	INTC_InstallINTCInterruptHandler(ISR_Pit, 59, 2); //PIT CH 0 INTC
	PIT.PITMCR.R = 0x00000001; 
	PIT.CH[0].LDVAL.B.TSV=0x0003E7FF;
	PIT.CH[0].TCTRL.B.TIE=1;
	PIT.CH[0].TCTRL.B.TEN=1;
	 // Init_PIT(0,64000000, main_timer_period);
#endif
	Os_EnableIrq();
	Rtm_Clear(&g_rtm_quick);
	Rtm_Init();
}



int main(void) {
  volatile int i = 0;
  init();
  //printf("alma");
  /* Loop forever */
  for (;;) {
    i++;
    Os_Scheduler(); // switch task, when one of the task finished
  }
}



