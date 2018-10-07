/*
 * FILE: main_p1.c 
 *
 * DESCRIPTION: Simple application for core_1.
 *  
 * Note that this program does NOT initialize the interrupt
 * controller or provide any service routines.
 *
 * Please see the projects in (CodeWarrior_Examples) for
 * more complete examples or consult the Qorivva cookbook AN2865.
 *
 * VERSION: 1.2 
 */
#include "aranyos.h"
#include "atomic.h"
#include "shared.h"

#pragma push
#pragma section code_type ".text_p1"
#pragma force_active on

int main_p1(void);
/*
TASK(TaskQuick_1)
{
	Atomic_Inc_u32(&g_counters[0]);
	Atomic_Inc_u32(&g_counters[2]);
}
*/
int main_p1(void) {
  volatile int i = 0;
 
  
  /* Loop forever */
  for (;;) {
    i++;
    if (!(i&0x00ff)) shared_task();
  }
}




#pragma pop
